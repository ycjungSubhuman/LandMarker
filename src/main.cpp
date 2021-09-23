#include <string>
#include <optional>
#include <iostream>

#include <Eigen/Core>
#include <Eigen/Dense>

#include <igl/unproject_onto_mesh.h>
#include <igl/opengl/glfw/Viewer.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>
#include <tinyobjloader/tiny_obj_loader.h>
#include <imgui/imgui.h>
#include <argparse/argparse.hpp>

#include "txtio.hpp"

struct Options {
    std::string path_obj;
    std::optional<std::string> path_input_landmark;
    std::string path_output;
};

Options read_options(int argc, char **argv)
{
    argparse::ArgumentParser program("LandMarker");
    program.add_argument("--mesh")
        .help("Path to the input .obj mesh file")
        .required();
    program.add_argument("--landmark_initial")
        .help("Path to initial landmark (optional)");
    program.add_argument("--output")
        .help("Path to output landmark file (optional. Default: output.txt)")
        .default_value(std::string("output.txt"));

    try{
        program.parse_args(argc, argv);
        Options result;
        result.path_obj = program.get<std::string>("--mesh");
        if(auto value = program.present<std::string>("--landmark_initial"))
        {
            result.path_input_landmark = *value;
        }
        result.path_output = program.get<std::string>("--output");

        return result;
    }
    catch (const std::runtime_error& err) {
        std::cout << err.what() << std::endl;
        std::cout << program;
        std::exit(1);
    }
}

void read_obj(const std::string &path, 
    Eigen::MatrixXd &V, Eigen::MatrixXd &C, Eigen::MatrixXi &F)
{
    tinyobj::ObjReaderConfig config;
    config.vertex_color = true;
    config.mtl_search_path = "./";

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(path, config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto &attr = reader.GetAttrib();
    auto &shapes = reader.GetShapes();

    const int count_vertex = attr.vertices.size() / 3;
    const int count_face = shapes[0].mesh.indices.size() / 3;
    V.resize(count_vertex, 3);
    C.resize(count_vertex, 3);
    F.resize(count_face, 3);

    for(int i=0; i<count_vertex; i++)
    {
        for(int k=0; k<3; k++)
        {
            V(i, k) = attr.vertices[3*i + k];
            C(i, k) = attr.colors[3*i + k];
        }
    }

    for(int i=0; i<count_face; i++)
    {
        for(int k=0; k<3; k++)
        {
            F(i, k) = shapes[0].mesh.indices[3*i + k].vertex_index;
        }
    }
}

int main(int argc, char **argv)
{
    Options options = read_options(argc, argv);

    auto menu = igl::opengl::glfw::imgui::ImGuiMenu();
    auto viewer = igl::opengl::glfw::Viewer();
    viewer.plugins.push_back(&menu);

    Eigen::MatrixXd V, C;
    Eigen::MatrixXi F;
    read_obj(options.path_obj, V, C, F);
    viewer.data().set_mesh(V, F);
    viewer.data().set_colors(C);
    std::vector<int> workspace_landmark;

    if(options.path_input_landmark.has_value())
    {
        Eigen::VectorXi landmarks_input = 
             readTxt<float>(*options.path_input_landmark).cast<int>(); // To read numpy.savetxt format, read in float then cast into int.

        std::cout << landmarks_input << std::endl;
        workspace_landmark.resize(landmarks_input.size());
        for(int i=0; i<landmarks_input.size(); i++)
        {
            workspace_landmark[i] = landmarks_input(i);
        }
    }

    auto update = [&](igl::opengl::glfw::Viewer &viewer) {
        viewer.data().clear();
        viewer.data().set_mesh(V, F);
        viewer.data().set_colors(C);

        for (size_t i = 0; i < workspace_landmark.size(); i++) {
            Eigen::RowVector3d pos =
                viewer.data().V.row(workspace_landmark[i]);

            Eigen::RowVector3d c;
            c << 0.0, 1.0, 0.0;

            Eigen::RowVector4f c_label;
            c_label << 0.0, 0.0, 0.0, 1.0;
            Eigen::RowVector3d offset_label;
            offset_label << 0.0, 0.0, 0.0;

            viewer.data().add_points(pos, c);
            viewer.data().add_label(pos + offset_label, std::to_string(i + 1));
            viewer.data().label_color = c_label;
        }
    };

    const auto back = [&]() {
        if (0 != workspace_landmark.size()) {
            workspace_landmark.pop_back();
        }
        update(viewer);
    };

    const auto save = [&]() {
        Eigen::VectorXi res = Eigen::Map<Eigen::VectorXi>(workspace_landmark.data(), workspace_landmark.size());
        writeTxt<int>(options.path_output, res);
    };

    const auto pick = [&]() {
        double x = viewer.current_mouse_x;
        double y = viewer.core().viewport(3) - viewer.current_mouse_y;
        int fid;
        bool val = false;
        Eigen::Vector3f bc;
        if (igl::unproject_onto_mesh(Eigen::Vector2f(x, y), viewer.core().view,
                                     viewer.core().proj, viewer.core().viewport,
                                     viewer.data().V, viewer.data().F, fid,
                                     bc)) {
            int vid = 0;
            bc.maxCoeff(&vid);
            workspace_landmark.push_back(F(fid, vid));
            val = true;
        }
        update(viewer);

        return val;
    };

    viewer.callback_mouse_down = [=](igl::opengl::glfw::Viewer &viewer,
                                     int button, int) -> bool {
        if (1 != button) {
            return false;
        }
        return pick();
    };

    menu.callback_draw_viewer_menu = [&]() {
        menu.draw_viewer_menu();
        if (ImGui::Button("Back")) {
            back();
        }
        if (ImGui::Button("Save")) {
            save();
        }
    };

    viewer.data().point_size = 10.0f;
    viewer.data().show_custom_labels = true;
    update(viewer);
    viewer.launch();

    return 0;
}
