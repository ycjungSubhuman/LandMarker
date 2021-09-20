#pragma once

#include <vector>
#include <stdexcept>
#include <fstream>
#include <Eigen/Dense>
#include <string>

template<typename T>
Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> readTxt(const std::string &p)
{
    using MatT = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;
    std::vector<std::vector<T>> buf;

    std::ifstream in(p);
    if(!in.good())
    {
        throw std::runtime_error("File does not exist: " + p);
    }

    int cnt=-1;
    int max_length = 0;
    for(std::string line; std::getline(in, line); )
    {
        if("" == line)
        {
            continue;
        }
        buf.push_back(std::vector<T>());
        cnt++;
        std::istringstream iss(line);
        T f;
        while(iss >> f)
        {
            buf[cnt].push_back(f);
        }
        max_length = (buf[cnt].size() > max_length) ?
            buf[cnt].size() :
            max_length;
    }

    if(buf.size() > 0)
    {
        MatT result(buf.size(), max_length);
        for(size_t i=0; i<buf.size(); i++)
        {
            if(buf[i].size() != max_length)
            {
                continue;
            }
            if(result.cols() != buf[i].size())
            {
                throw std::runtime_error("Non-uniform column detected");
            }
            result.row(i) =
                Eigen::Map<Eigen::Matrix<T, 1, Eigen::Dynamic>>
                (buf[i].data(), buf[i].size());
        }
        return result;
    }
    else
    {
        return MatT();
    }
}

    template<typename T>
void writeTxt(
        const std::string &p,
        const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> &mat)
{
    std::ofstream out(p);
    if(!out.good())
    {
        throw std::runtime_error("Cannot create file: " + p);
    }

    for(Eigen::Index i=0; i<mat.rows(); i++)
    {
        for(Eigen::Index j=0; j<mat.cols(); j++)
        {
            out << mat(i, j);
            if(j+1 == mat.cols())
            {
                out << "\n";
            }
            else
            {
                out << " ";
            }
        }
    }
}
