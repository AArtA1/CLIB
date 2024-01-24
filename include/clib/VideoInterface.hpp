#pragma once

#include "ImgView.hpp"
#include "VideoView.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace vsim
{
/*!
 * \brief Класс для связи Verilog-интерфейса и кода на C++
 *
 * \details
 */
using std::vector;
using idx_t = clib::VideoView::idx_t;
using pixel_t = clib::VideoView::pixel_t;

/*!
 * \brief General media class. Can contain video and image
 *
 * \details If number of frames = 2, the video_ array contains two copies of the image. If number of frames > 2, the
 *          video_ array contains the video. Number of frames can not be less than 2
 */
class Video
{
    /// video_[i][j][k][l]
    /// i - frame number
    /// j - column number
    /// k - row number
    /// l - color number
    vector<vector<vector<vector<pixel_t>>>> video_;

    // /Equal to 0 for image
    size_t fps_;

  public:
    Video() = default;

    /*! @brief Automatically detects the file type and fills the video_ array
     *
     * \param[in] path Path to the file
     */
    Video(const std::string &path)
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
            fps_ = clib::get_fps(path);
        }
        else if (clib::check_ext(path, clib::image_extensions))
        {
            img_view.read_img(path);
            frames = 2;
            rows = img_view.rows();
            cols = img_view.cols();
            clrs = img_view.clrs();

            is_video = false;
            fps_ = 0;
        }
        else
            assert(0);

        assert(rows > 0);
        assert(cols > 0);
        assert(clrs > 0);
        assert(frames > 1);

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
                            // The second frame must contain a copy of the first frame
                            if (i == 1)
                                video_[i][j][k][l] = video_[0][j][k][l];
                            else
                                video_[i][j][k][l] = img_view.get(j, k, l);
                    }
                }
            }
        }
    }

    /*! @brief Creates video_ array with the specified dimensions
     *
     * \param[in] frames - number of frames
     * \param[in] rows - number of rows
     * \param[in] cols - number of columns
     * \param[in] clrs - number of colors
     * \param[in] fps - fps of the video. For image can be omitted
     */
    Video(idx_t frames, idx_t rows, idx_t cols, idx_t clrs, size_t fps = 0)
    {
        assert(frames > 1);

        fps_ = fps;
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

    /*! @brief Writes image to the file
     *
     * \param[in] path - file path
     */
    void write(const std::string &path)
    {
        auto vid_view = clib::CVideoView{};
        auto img_view = clib::CImgView{};
        bool is_video = false;

        if (frames() > 2)
        {
            vid_view.init(rows(), cols(), clrs(), frames(), fps_);
            is_video = true;
        }
        else if (frames() == 2)
        {
            img_view.init(rows(), cols(), clrs());
            is_video = false;
        }
        else
            assert(0);

        // Optimization for image. The second frame must contain a copy of the first frame
        idx_t i = 0;
        if (!is_video)
            i = 1;

        for (; i < frames(); ++i)
            for (idx_t j = 0; j < rows(); ++j)
                for (idx_t k = 0; k < cols(); ++k)
                    for (idx_t l = 0; l < clrs(); ++l)
                    {
                        if (is_video)
                            vid_view.set(video_[i][j][k][l], j, k, l, i);
                        else
                            img_view.set(video_[i][j][k][l], j, k, l);
                    }

        if (is_video)
            vid_view.write_video(path);
        else
            img_view.write_img(path);
    }

    /// Number of frames
    idx_t frames() const
    {
        // static_cast due to different
        // Cimg library index size = unsigned
        //         and vector size = long unsigned
        return static_cast<unsigned>(video_.size());
    }

    /// Height
    idx_t rows() const
    {
        // static_cast due to different
        // Cimg library index size = unsigned
        //         and vector size = long unsigned
        assert(video_.size() > 0);
        return static_cast<unsigned>(video_[0].size());
    }

    /// Width
    idx_t cols() const
    {
        // static_cast due to different
        // Cimg library index size = unsigned
        //         and vector size = long unsigned
        assert(video_.size() > 0);
        return static_cast<unsigned>(video_[0][0].size());
    }

    /// Number of colors
    idx_t clrs() const
    {
        // static_cast due to different
        // Cimg library index size = unsigned
        //         and vector size = long unsigned
        assert(video_.size() > 0);
        return static_cast<unsigned>(video_[0][0][0].size());
    }

    /// Number of fps
    size_t fps() const
    {
        return fps_;
    }

    /// Get pixel
    pixel_t get(idx_t frame, idx_t i, idx_t j, idx_t clr) const
    {
        return video_[frame][i][j][clr];
    }

    /// Set pixel
    void set(pixel_t val, idx_t frame, idx_t i, idx_t j, idx_t clr)
    {
        video_[frame][i][j][clr] = val;
    }
};

class Serializer
{
    Video video_;

    /// Points to the pixel that will be popped out
    /// At the beginning points to the first pixel
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

    /// @brief Automatically detects the file type and creates the Video
    /// @param[in] path - file path
    void init(const std::string &path)
    {
        video_ = Video(path);
        cur_frame_ = 0;
        cur_row_ = 0;
        cur_col_ = 0;
        cur_clr_ = 0;

        is_inited_ = true;
    }

    /// @brief Creates a Video with the specified dimensions
    /// @param[in] frames - number of frames
    /// @param[in] rows - number of rows
    /// @param[in] cols - number of columns
    /// @param[in] clrs - number of colors
    /// @param[in] fps - fps of the video. For image can be omitted
    void init(idx_t frames, idx_t rows, idx_t cols, idx_t clrs, size_t fps = 0)
    {
        video_ = Video(frames, rows, cols, clrs, fps);
        cur_frame_ = 0;
        cur_row_ = 0;
        cur_col_ = 0;
        cur_clr_ = 0;

        is_inited_ = true;
    }

    /// Number of frames
    idx_t frames() const
    {
        return video_.frames();
    }

    /// Height
    idx_t rows() const
    {
        return video_.rows();
    }

    /// Width
    idx_t cols() const
    {

        return video_.cols();
    }

    /// Number of colors
    idx_t clrs() const
    {
        return video_.clrs();
    }

    /// Number of fps
    size_t fps() const
    {
        return video_.fps();
    }

    idx_t frame() const
    {
        return cur_frame_;
    }
    idx_t row() const
    {
        return cur_row_;
    }
    idx_t col() const
    {
        return cur_col_;
    }
    idx_t clr() const
    {
        return cur_clr_;
    }
    /// @brief Get pixel from Video. Starts popping out pixels from the beginning of image
    ///        Returns 0 if all pixels are popped out
    /// @return pixel
    pixel_t pop()
    {
        assert(is_inited_);

        if (cur_frame_ >= video_.frames())
        {
            std::cerr << "frames() = " << frames() << "; cur_frame = " << cur_frame_ << std::endl;
            std::cerr << "rows() =   " << rows() << "; cur_row   = " << cur_row_ << std::endl;
            std::cerr << "cols() =   " << cols() << "; cur_col   = " << cur_col_ << std::endl;
            std::cerr << "clrs() =   " << clrs() << "; cur_clr   = " << cur_clr_ << std::endl;

            return 0;
        }
        pixel_t pix = video_.get(cur_frame_, cur_row_, cur_col_, cur_clr_);

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

        return pix;
    }
};

class Deserializer
{
    Video video_;

    /// Points to the pixel that will be pushed in
    /// At the beginning points to the first pixel
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

    /// @brief Creates a Video with the specified dimensions
    /// @param[in] frames - number of frames
    /// @param[in] rows - number of rows
    /// @param[in] cols - number of columns
    /// @param[in] clrs - number of colors
    /// @param[in] fps - fps of the video. For image can be omitted
    void init(idx_t frames, idx_t rows, idx_t cols, idx_t clrs, size_t fps = 0)
    {
        video_ = Video(frames, rows, cols, clrs, fps);
        cur_frame_ = 0;
        cur_row_ = 0;
        cur_col_ = 0;
        cur_clr_ = 0;

        is_inited_ = true;
    }

    /// @brief Automatically detects the file type and creates the Video
    /// @param[in] path - file path
    void init(const std::string &path)
    {
        video_ = Video(path);
        cur_frame_ = 0;
        cur_row_ = 0;
        cur_col_ = 0;
        cur_clr_ = 0;

        is_inited_ = true;
    }

    /// @brief Writes Video to the  file
    /// @param[in] path - file path
    void write(const std::string &path)
    {
        assert(is_inited_);
        video_.write(path);
    }

    /// Number of frames
    idx_t frames() const
    {
        return video_.frames();
    }

    /// Height
    idx_t rows() const
    {
        return video_.rows();
    }

    /// Width
    idx_t cols() const
    {

        return video_.cols();
    }

    /// Number of colors
    idx_t clrs() const
    {
        return video_.clrs();
    }

    /// Number of fps
    size_t fps() const
    {
        return video_.fps();
    }

    idx_t frame() const
    {
        return cur_frame_;
    }
    idx_t row() const
    {
        return cur_row_;
    }
    idx_t col() const
    {
        return cur_col_;
    }
    idx_t clr() const
    {
        return cur_clr_;
    }

    /// @brief Set pixel in Video. Starts pushing in pixels from the beginning of image
    ///        Does nothing if all pixels are pushed in
    /// @param[in] pixel
    void push(pixel_t pixel)
    {
        assert(is_inited_);

        if (cur_frame_ >= video_.frames())
        {
            std::cerr << "frames() = " << frames() << "; cur_frame = " << cur_frame_ << std::endl;
            std::cerr << "rows() =   " << rows() << "; cur_row   = " << cur_row_ << std::endl;
            std::cerr << "cols() =   " << cols() << "; cur_col   = " << cur_col_ << std::endl;
            std::cerr << "clrs() =   " << clrs() << "; cur_clr   = " << cur_clr_ << std::endl;

            return;
        }
        video_.set(pixel, cur_frame_, cur_row_, cur_col_, cur_clr_);

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
    }
};

struct VideoInterface
{
    Serializer source;
    Deserializer target;

    VideoInterface() = default;

    /// @brief Pop pixel. Starts from the beginning
    /// @return pixel
    pixel_t pop()
    {
        return source.pop();
    }

    /// @brief Push pixel. Starts from the beginning
    /// @param pixel
    void push(pixel_t pixel)
    {
        target.push(pixel);
    }
};

} // namespace vsim
