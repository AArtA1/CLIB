#include "clib/VideoView.hpp"
#include <limits>

namespace clib
{

using idx_t = VideoView::idx_t;
using pixel_t = VideoView::pixel_t;

idx_t CVideoView::rows() const
{
    check_created();
    return static_cast<idx_t>(video_.height());
}
idx_t CVideoView::cols() const
{
    check_created();
    return static_cast<idx_t>(video_.width());
}
idx_t CVideoView::clrs() const
{
    check_created();
    return static_cast<idx_t>(video_.spectrum());
}
idx_t CVideoView::frames() const
{
    check_created();
    return static_cast<idx_t>(video_.depth());
}

// CImg stores data as [width,height]. Therefore, the data in cimg are transposed
pixel_t CVideoView::get(idx_t i, idx_t j, idx_t clr, idx_t frame) const
{
    assert(i < rows());
    assert(j < cols());
    assert(clr < clrs());
    assert(frame < frames());

    check_created();
    return video_(j, i, frame, clr);
}

// CImg stores data as [width,height]. Therefore, the data in cimg are transposed
void CVideoView::set(pixel_t val, idx_t i, idx_t j, idx_t clr, idx_t frame)
{
    assert(i < rows());
    assert(j < cols());
    assert(clr < clrs());
    assert(frame < frames());

    check_created();
    video_(j, i, frame, clr) = val;
}

void CVideoView::read_video(const std::string &path)
{
    if (check_ext(path, video_extensions))
    {
        video_ = cimg_library::CImg<pixel_t>::get_load_video(path.c_str());
        fps_ = get_fps(path);
    }
    else
    {
        throw std::invalid_argument("Unknown format: " + path);
    }

    video_created_ = true;
}

void CVideoView::write_video(const std::string &path)
{
    if (check_ext(path, video_extensions))
        video_.save_video(path.c_str(), static_cast<unsigned int>(fps_));
    else
        throw std::invalid_argument("Unknown format: " + path);
}

bool CVideoView::check_ext(const std::string &s, const std::vector<std::string> &exts)
{
    for (auto ext : exts)
        if (s.substr(s.find_last_of(".") + 1) == ext)
            return true;
    return false;
}

void CVideoView::check_created() const
{
    if (!video_created_)
        throw std::runtime_error{"video was not created"};
}

size_t get_fps(const std::string &path)
{
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    av_register_all();
    #pragma GCC diagnostic warning "-Wdeprecated-declarations"


    // Input video file path
    const char *videoFilePath = path.c_str();

    // Open the video file
    AVFormatContext *formatContext = avformat_alloc_context();
    if (avformat_open_input(&formatContext, videoFilePath, nullptr, nullptr) != 0)
        throw std::runtime_error("Error opening video file");

    // Retrieve stream information
    if (avformat_find_stream_info(formatContext, nullptr) < 0)
        throw std::runtime_error("Error finding stream information");
            

    // Find the first video stream
    int videoStreamIndex = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; i++)
    {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            assert(i < std::numeric_limits<int>::max());
            videoStreamIndex = static_cast<int>(i);
            break;
        }
    }

    if (videoStreamIndex == -1)
    {
        avformat_close_input(&formatContext);
        throw std::runtime_error("No video stream found in the input file");
    }

    // Get the frame rate of the video stream
    AVRational frameRate = formatContext->streams[videoStreamIndex]->avg_frame_rate;
    size_t fps = static_cast<size_t>(av_q2d(frameRate));

    // Close the video file
    avformat_close_input(&formatContext);

    return fps;
}

} // namespace clib