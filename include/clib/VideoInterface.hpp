#pragma once

#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <VideoView.hpp>
#include <ImgView.hpp>

namespace vsim
{
/*!
 * \brief Класс для связи Verilog-интерфейса и кода на C++
 *
 * \details 
 */
using std::vector;
using idx_t = int;
using pixel_t = int;

class Video
{
    vector<vector<vector<vector<pixel_t>>>> video_;

public:

    Video() = default;

    Video(const std::string& path)
    {
        idx_t frames = 0;
        idx_t rows = 0;
        idx_t cols = 0;
        idx_t clrs = 0;

        auto vid_view = clib::CVideoView{};
        auto img_view = clib::CImgView{};

        bool is_video = false;

        if (clib::check_ext(path, clib::video_extensions))
        {
            vid_view.read_video(path);
            frames = vid_view.frames();
            rows = vid_view.rows();
            cols = vid_view.cols();
            clrs = vid_view.clrs();

            is_video = true;
        }
        else if (clib::check_ext(path, clib::image_extensions))
        {
            img_view.read_img(path);
            frames = 1;
            rows = img_view.rows();
            cols = img_view.cols();
            clrs = img_view.clrs();

            is_video = false;
        }
        else
            assert(0);

        assert(rows > 0);
        assert(cols > 0);
        assert(clrs > 0);
        assert(frames > 0);

        video_.reserve(frames);
        video_.resize(frames);
        for (idx_t i = 0; i < frames; ++i)
        {
            video_[i].reserve(rows);
            video_[i].resize(rows);
            for (idx_t j = 0; j < rows; ++j)
            {
                video_[i][j].reserve(cols);
                video_[i][j].resize(cols);
                for (idx_t k = 0; k < cols; ++k)
                {
                    video_[i][j][k].reserve(clrs);
                    video_[i][j][k].resize(clrs);

                    for (idx_t l = 0; l < clrs; ++l)
                    {
                        if (is_video)
                            video_[i][j][k][l] = vid_view.get(j, k, l, i);
                        else
                            video_[i][j][k][l] = img_view.get(j, k, l);
                    }
                        
                }
            }
        }
    }

    Video(idx_t frames, idx_t rows, idx_t cols, idx_t clrs)
    {
        assert(frames > 0);

        video_.reserve(frames);
        video_.resize(frames);
        for (idx_t i = 0; i < frames; ++i)
        {
            video_[i].reserve(rows);
            video_[i].resize(rows);
            for (idx_t j = 0; j < rows; ++j)
            {
                video_[i][j].reserve(cols);
                video_[i][j].resize(cols);
                for (idx_t k = 0; k < cols; ++k)
                {
                    video_[i][j][k].reserve(clrs);
                    video_[i][j][k].resize(clrs);
                }
            }
        }
    }

    void write(const std::string& path)
    {
        auto vid_view = clib::CVideoView{};
        auto img_view = clib::CImgView{};
        bool is_video = false;

        if (frames() > 2)
        {
            vid_view.init(rows(), cols(), clrs(), frames());
            is_video = true;
        }
        else if (frames() == 2)
        {
            img_view.init(rows(), cols(), clrs());
            is_video = false;
        }
        else
            assert(0);

        idx_t i = 0;
        if (!is_video)
            idx_t i = 1;

        for (; i < frames(); ++i)
            for (idx_t j = 0; j < rows(); ++j)
                for (idx_t k = 0; k < cols(); ++k)
                    for (idx_t l = 0; l < clrs(); ++l)
                    {
                        auto val = video_[i][j][k][l];
                        if (is_video)
                            vid_view.set(video_[i][j][k][l], j, k, l, i);
                        else
                            img_view.set(video_[i][j][k][l], j, k, l);
                    }
    }

    // количество кадров
    idx_t frames() const
    {
        return video_.size();
    }

    // height
    idx_t rows() const
    {
        assert(video_.size() > 0);
        return video_[0].size();
    }
    
    // width
    idx_t cols() const
    {
        assert(video_.size() > 0);
        return video_[0][0].size();
    }
    
    // количество цветов
    idx_t clrs() const
    {
        assert(video_.size() > 0);
        return video_[0][0][0].size();
    }

    pixel_t get(idx_t frame, idx_t i, idx_t j, idx_t clr) const
    {
        return video_[frame][i][j][clr];
    }
    void set(pixel_t val, idx_t frame, idx_t i, idx_t j, idx_t clr)
    {
        video_[frame][i][j][clr] = val;
    }
};

class Serializer
{
    Video video_;

    idx_t cur_frame_;
    idx_t cur_row_;
    idx_t cur_col_;
    idx_t cur_clr_;

    bool is_inited_;
public:

    Serializer()
    {
        is_inited_ = false;
    }

    void init(const std::string& path)
    {
        video_ = Video(path);
        cur_frame_ = -1;
        cur_row_ = video_.rows() - 1;
        cur_col_ = video_.cols() - 1;
        cur_clr_ = video_.clrs() - 1;

        is_inited_ = true;
    }

    void init(idx_t frames, idx_t rows, idx_t cols, idx_t clrs)
    {
        video_ = Video(frames, rows, cols, clrs);
        cur_frame_ = -1;
        cur_row_ = rows - 1;
        cur_col_ = cols - 1;
        cur_clr_ = clrs - 1;

        is_inited_ = true;
    }

    pixel_t pop()
    {
        assert(is_inited_);

        if (cur_clr_ < 0)
        {
            cur_clr_ = video_.clrs()-1;
            cur_col_ -= 1;
        }
        if (cur_col_ < 0)
        {
            cur_col_ = video_.cols()-1;
            cur_row_ -= 1;
        }
        if (cur_row_ < 0)
        {
            cur_row_ = video_.rows()-1;
            cur_frame_ -= 1;
        }
        if (cur_frame_ < 0)
        {
            throw std::runtime_error{"Index out of range"};
        }
        cur_clr_--;

        return video_.get(cur_frame_, cur_row_, cur_col_, cur_clr_ + 1);
    }
};

class Deserializer
{
    Video video_;

    idx_t cur_frame_;
    idx_t cur_row_;
    idx_t cur_col_;
    idx_t cur_clr_;

    bool is_inited_;
public:

    Deserializer()
    {
        is_inited_ = false;
    }

    void init(idx_t frames, idx_t rows, idx_t cols, idx_t clrs)
    {
        video_ = Video(frames, rows, cols, clrs);
        cur_frame_ = -1;
        cur_row_ = rows - 1;
        cur_col_ = cols - 1;
        cur_clr_ = clrs - 1;

        is_inited_ = true;
    }

    void write(const std::string& path)
    {
        assert(is_inited_);

        video_.write(path);
    }
    
    void push(pixel_t pixel)
    {
        assert(is_inited_);

        cur_clr_++;
        if (cur_clr_ >= video_.clrs())
        {
            cur_clr_ = 0;
            cur_col_ += 1;
        }
        if (cur_col_ >= video_.cols())
        {
            cur_col_ = 0;
            cur_row_ += 1;
        }
        if (cur_row_ >= video_.rows())
        {
            cur_row_ = 0;
            cur_frame_ += 1;
        }
        if (cur_frame_ >= video_.frames())
        {
            throw std::runtime_error{"Index out of range"};
        }

        video_.set(pixel, cur_frame_, cur_row_, cur_col_, cur_clr_);
    }
};

struct VideoInterface
{
    Serializer source;
    Deserializer target;

    VideoInterface() = default;

    pixel_t pop()
    {
        return source.pop();
    }
    void push(pixel_t pixel)
    {
        target.push(pixel);
    }
};

} // namespace clib
