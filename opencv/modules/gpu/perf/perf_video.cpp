#include "perf_precomp.hpp"

#ifdef HAVE_CUDA

//////////////////////////////////////////////////////
// BroxOpticalFlow

GPU_PERF_TEST_1(BroxOpticalFlow, cv::gpu::DeviceInfo)
{
    cv::gpu::DeviceInfo devInfo = GetParam();
    cv::gpu::setDevice(devInfo.deviceID());

    cv::Mat frame0_host = readImage("gpu/opticalflow/frame0.png", cv::IMREAD_GRAYSCALE);
    ASSERT_FALSE(frame0_host.empty());

    cv::Mat frame1_host = readImage("gpu/opticalflow/frame1.png", cv::IMREAD_GRAYSCALE);
    ASSERT_FALSE(frame1_host.empty());

    frame0_host.convertTo(frame0_host, CV_32FC1, 1.0 / 255.0);
    frame1_host.convertTo(frame1_host, CV_32FC1, 1.0 / 255.0);

    cv::gpu::GpuMat frame0(frame0_host);
    cv::gpu::GpuMat frame1(frame1_host);
    cv::gpu::GpuMat u;
    cv::gpu::GpuMat v;

    cv::gpu::BroxOpticalFlow d_flow(0.197f /*alpha*/, 50.0f /*gamma*/, 0.8f /*scale_factor*/,
                                    10 /*inner_iterations*/, 77 /*outer_iterations*/, 10 /*solver_iterations*/);

    d_flow(frame0, frame1, u, v);

    declare.time(10);

    TEST_CYCLE()
    {
        d_flow(frame0, frame1, u, v);
    }
}

INSTANTIATE_TEST_CASE_P(Video, BroxOpticalFlow, ALL_DEVICES);

//////////////////////////////////////////////////////
// InterpolateFrames

GPU_PERF_TEST_1(InterpolateFrames, cv::gpu::DeviceInfo)
{
    cv::gpu::DeviceInfo devInfo = GetParam();
    cv::gpu::setDevice(devInfo.deviceID());

    cv::Mat frame0_host = readImage("gpu/perf/aloe.jpg", cv::IMREAD_GRAYSCALE);
    ASSERT_FALSE(frame0_host.empty());

    cv::Mat frame1_host = readImage("gpu/perf/aloeR.jpg", cv::IMREAD_GRAYSCALE);
    ASSERT_FALSE(frame1_host.empty());

    frame0_host.convertTo(frame0_host, CV_32FC1, 1.0 / 255.0);
    frame1_host.convertTo(frame1_host, CV_32FC1, 1.0 / 255.0);

    cv::gpu::GpuMat frame0(frame0_host);
    cv::gpu::GpuMat frame1(frame1_host);
    cv::gpu::GpuMat fu, fv;
    cv::gpu::GpuMat bu, bv;

    cv::gpu::BroxOpticalFlow d_flow(0.197f /*alpha*/, 50.0f /*gamma*/, 0.8f /*scale_factor*/,
                                    10 /*inner_iterations*/, 77 /*outer_iterations*/, 10 /*solver_iterations*/);

    d_flow(frame0, frame1, fu, fv);
    d_flow(frame1, frame0, bu, bv);

    cv::gpu::GpuMat newFrame;
    cv::gpu::GpuMat buf;

    cv::gpu::interpolateFrames(frame0, frame1, fu, fv, bu, bv, 0.5f, newFrame, buf);

    TEST_CYCLE()
    {
        cv::gpu::interpolateFrames(frame0, frame1, fu, fv, bu, bv, 0.5f, newFrame, buf);
    }
}

INSTANTIATE_TEST_CASE_P(Video, InterpolateFrames, ALL_DEVICES);

//////////////////////////////////////////////////////
// CreateOpticalFlowNeedleMap

GPU_PERF_TEST_1(CreateOpticalFlowNeedleMap, cv::gpu::DeviceInfo)
{
    cv::gpu::DeviceInfo devInfo = GetParam();
    cv::gpu::setDevice(devInfo.deviceID());

    cv::Mat frame0_host = readImage("gpu/perf/aloe.jpg", cv::IMREAD_GRAYSCALE);
    ASSERT_FALSE(frame0_host.empty());

    cv::Mat frame1_host = readImage("gpu/perf/aloeR.jpg", cv::IMREAD_GRAYSCALE);
    ASSERT_FALSE(frame1_host.empty());

    frame0_host.convertTo(frame0_host, CV_32FC1, 1.0 / 255.0);
    frame1_host.convertTo(frame1_host, CV_32FC1, 1.0 / 255.0);

    cv::gpu::GpuMat frame0(frame0_host);
    cv::gpu::GpuMat frame1(frame1_host);
    cv::gpu::GpuMat u, v;

    cv::gpu::BroxOpticalFlow d_flow(0.197f /*alpha*/, 50.0f /*gamma*/, 0.8f /*scale_factor*/,
                                    10 /*inner_iterations*/, 77 /*outer_iterations*/, 10 /*solver_iterations*/);

    d_flow(frame0, frame1, u, v);

    cv::gpu::GpuMat vertex, colors;

    cv::gpu::createOpticalFlowNeedleMap(u, v, vertex, colors);

    TEST_CYCLE()
    {
        cv::gpu::createOpticalFlowNeedleMap(u, v, vertex, colors);
    }
}

INSTANTIATE_TEST_CASE_P(Video, CreateOpticalFlowNeedleMap, ALL_DEVICES);

//////////////////////////////////////////////////////
// GoodFeaturesToTrack

IMPLEMENT_PARAM_CLASS(MinDistance, double)

GPU_PERF_TEST(GoodFeaturesToTrack, cv::gpu::DeviceInfo, MinDistance)
{
    cv::gpu::DeviceInfo devInfo = GET_PARAM(0);
    cv::gpu::setDevice(devInfo.deviceID());

    double minDistance = GET_PARAM(1);

    cv::Mat image_host = readImage("gpu/perf/aloe.jpg", cv::IMREAD_GRAYSCALE);
    ASSERT_FALSE(image_host.empty());

    cv::gpu::GoodFeaturesToTrackDetector_GPU detector(8000, 0.01, minDistance);

    cv::gpu::GpuMat image(image_host);
    cv::gpu::GpuMat pts;

    detector(image, pts);

    TEST_CYCLE()
    {
        detector(image, pts);
    }
}

INSTANTIATE_TEST_CASE_P(Video, GoodFeaturesToTrack, testing::Combine(
    ALL_DEVICES,
    testing::Values(MinDistance(0.0), MinDistance(3.0))));

//////////////////////////////////////////////////////
// PyrLKOpticalFlowSparse

IMPLEMENT_PARAM_CLASS(GraySource, bool)
IMPLEMENT_PARAM_CLASS(Points, int)
IMPLEMENT_PARAM_CLASS(WinSize, int)
IMPLEMENT_PARAM_CLASS(Levels, int)
IMPLEMENT_PARAM_CLASS(Iters, int)

GPU_PERF_TEST(PyrLKOpticalFlowSparse, cv::gpu::DeviceInfo, GraySource, Points, WinSize, Levels, Iters)
{
    cv::gpu::DeviceInfo devInfo = GET_PARAM(0);
    cv::gpu::setDevice(devInfo.deviceID());

    bool useGray = GET_PARAM(1);
    int points = GET_PARAM(2);
    int winSize = GET_PARAM(3);
    int levels = GET_PARAM(4);
    int iters = GET_PARAM(5);

    cv::Mat frame0_host = readImage("gpu/opticalflow/frame0.png", useGray ? cv::IMREAD_GRAYSCALE : cv::IMREAD_COLOR);
    ASSERT_FALSE(frame0_host.empty());

    cv::Mat frame1_host = readImage("gpu/opticalflow/frame1.png", useGray ? cv::IMREAD_GRAYSCALE : cv::IMREAD_COLOR);
    ASSERT_FALSE(frame1_host.empty());

    cv::Mat gray_frame;
    if (useGray)
        gray_frame = frame0_host;
    else
        cv::cvtColor(frame0_host, gray_frame, cv::COLOR_BGR2GRAY);

    cv::gpu::GpuMat pts;

    cv::gpu::GoodFeaturesToTrackDetector_GPU detector(points, 0.01, 0.0);
    detector(cv::gpu::GpuMat(gray_frame), pts);

    cv::gpu::PyrLKOpticalFlow pyrLK;
    pyrLK.winSize = cv::Size(winSize, winSize);
    pyrLK.maxLevel = levels - 1;
    pyrLK.iters = iters;

    cv::gpu::GpuMat frame0(frame0_host);
    cv::gpu::GpuMat frame1(frame1_host);
    cv::gpu::GpuMat nextPts;
    cv::gpu::GpuMat status;

    pyrLK.sparse(frame0, frame1, pts, nextPts, status);

    TEST_CYCLE()
    {
        pyrLK.sparse(frame0, frame1, pts, nextPts, status);
    }
}

INSTANTIATE_TEST_CASE_P(Video, PyrLKOpticalFlowSparse, testing::Combine(
    ALL_DEVICES,
    testing::Values(GraySource(true), GraySource(false)),
    testing::Values(Points(1000), Points(2000), Points(4000), Points(8000)),
    testing::Values(WinSize(9), WinSize(13), WinSize(17), WinSize(21)),
    testing::Values(Levels(1), Levels(2), Levels(3)),
    testing::Values(Iters(1), Iters(10), Iters(30))));

//////////////////////////////////////////////////////
// PyrLKOpticalFlowDense

GPU_PERF_TEST(PyrLKOpticalFlowDense, cv::gpu::DeviceInfo, WinSize, Levels, Iters)
{
    cv::gpu::DeviceInfo devInfo = GET_PARAM(0);
    cv::gpu::setDevice(devInfo.deviceID());

    int winSize = GET_PARAM(1);
    int levels = GET_PARAM(2);
    int iters = GET_PARAM(3);

    cv::Mat frame0_host = readImage("gpu/opticalflow/frame0.png", cv::IMREAD_GRAYSCALE);
    ASSERT_FALSE(frame0_host.empty());

    cv::Mat frame1_host = readImage("gpu/opticalflow/frame1.png", cv::IMREAD_GRAYSCALE);
    ASSERT_FALSE(frame1_host.empty());

    cv::gpu::GpuMat frame0(frame0_host);
    cv::gpu::GpuMat frame1(frame1_host);
    cv::gpu::GpuMat u;
    cv::gpu::GpuMat v;

    cv::gpu::PyrLKOpticalFlow pyrLK;

    pyrLK.winSize = cv::Size(winSize, winSize);
    pyrLK.maxLevel = levels - 1;
    pyrLK.iters = iters;

    pyrLK.dense(frame0, frame1, u, v);

    declare.time(30);

    TEST_CYCLE()
    {
        pyrLK.dense(frame0, frame1, u, v);
    }
}

INSTANTIATE_TEST_CASE_P(Video, PyrLKOpticalFlowDense, testing::Combine(
    ALL_DEVICES,
    testing::Values(WinSize(3), WinSize(5), WinSize(7), WinSize(9), WinSize(13), WinSize(17), WinSize(21)),
    testing::Values(Levels(1), Levels(2), Levels(3)),
    testing::Values(Iters(1), Iters(10))));

//////////////////////////////////////////////////////
// FarnebackOpticalFlowTest

GPU_PERF_TEST_1(FarnebackOpticalFlowTest, cv::gpu::DeviceInfo)
{
    cv::gpu::DeviceInfo devInfo = GetParam();
    cv::gpu::setDevice(devInfo.deviceID());

    cv::Mat frame0_host = readImage("gpu/opticalflow/frame0.png", cv::IMREAD_GRAYSCALE);
    ASSERT_FALSE(frame0_host.empty());

    cv::Mat frame1_host = readImage("gpu/opticalflow/frame1.png", cv::IMREAD_GRAYSCALE);
    ASSERT_FALSE(frame1_host.empty());

    cv::gpu::GpuMat frame0(frame0_host);
    cv::gpu::GpuMat frame1(frame1_host);
    cv::gpu::GpuMat u;
    cv::gpu::GpuMat v;

    cv::gpu::FarnebackOpticalFlow farneback;

    farneback(frame0, frame1, u, v);

    declare.time(10);

    TEST_CYCLE()
    {
        farneback(frame0, frame1, u, v);
    }
}

INSTANTIATE_TEST_CASE_P(Video, FarnebackOpticalFlowTest, ALL_DEVICES);

//////////////////////////////////////////////////////
// FGDStatModel

GPU_PERF_TEST(FGDStatModel, cv::gpu::DeviceInfo, std::string)
{
    cv::gpu::DeviceInfo devInfo = GET_PARAM(0);
    cv::gpu::setDevice(devInfo.deviceID());

    std::string inputFile = perf::TestBase::getDataPath(std::string("gpu/video/") + GET_PARAM(1));

    cv::VideoCapture cap(inputFile);
    ASSERT_TRUE(cap.isOpened());

    cv::Mat frame;
    cap >> frame;
    ASSERT_FALSE(frame.empty());

    cv::gpu::GpuMat d_frame(frame);
    cv::gpu::FGDStatModel d_model(4);
    d_model.create(d_frame);

    declare.time(10);

    for (int i = 0; i < 10; ++i)
    {
        cap >> frame;
        ASSERT_FALSE(frame.empty());

        d_frame.upload(frame);

        startTimer(); next();
        d_model.update(d_frame);
        stopTimer();
    }
}

INSTANTIATE_TEST_CASE_P(Video, FGDStatModel, testing::Combine(
    ALL_DEVICES,
    testing::Values(std::string("768x576.avi"), std::string("1920x1080.avi"))));

//////////////////////////////////////////////////////
// MOG

IMPLEMENT_PARAM_CLASS(LearningRate, double)

GPU_PERF_TEST(MOG, cv::gpu::DeviceInfo, std::string, Channels, LearningRate)
{
    cv::gpu::DeviceInfo devInfo = GET_PARAM(0);
    cv::gpu::setDevice(devInfo.deviceID());

    std::string inputFile = perf::TestBase::getDataPath(std::string("gpu/video/") + GET_PARAM(1));
    int cn = GET_PARAM(2);
    double learningRate = GET_PARAM(3);

    cv::VideoCapture cap(inputFile);
    ASSERT_TRUE(cap.isOpened());

    cv::Mat frame;

    cv::gpu::GpuMat d_frame;
    cv::gpu::MOG_GPU mog;
    cv::gpu::GpuMat foreground;

    cap >> frame;
    ASSERT_FALSE(frame.empty());

    if (cn != 3)
    {
        cv::Mat temp;
        if (cn == 1)
            cv::cvtColor(frame, temp, cv::COLOR_BGR2GRAY);
        else
            cv::cvtColor(frame, temp, cv::COLOR_BGR2BGRA);
        cv::swap(temp, frame);
    }

    d_frame.upload(frame);

    mog(d_frame, foreground, learningRate);

    for (int i = 0; i < 10; ++i)
    {
        cap >> frame;
        ASSERT_FALSE(frame.empty());

        if (cn != 3)
        {
            cv::Mat temp;
            if (cn == 1)
                cv::cvtColor(frame, temp, cv::COLOR_BGR2GRAY);
            else
                cv::cvtColor(frame, temp, cv::COLOR_BGR2BGRA);
            cv::swap(temp, frame);
        }

        d_frame.upload(frame);

        startTimer(); next();
        mog(d_frame, foreground, learningRate);
        stopTimer();
    }
}

INSTANTIATE_TEST_CASE_P(Video, MOG, testing::Combine(
    ALL_DEVICES,
    testing::Values(std::string("768x576.avi"), std::string("1920x1080.avi")),
    testing::Values(Channels(1), Channels(3), Channels(4)),
    testing::Values(LearningRate(0.0), LearningRate(0.01))));

//////////////////////////////////////////////////////
// MOG2

GPU_PERF_TEST(MOG2_update, cv::gpu::DeviceInfo, std::string, Channels)
{
    cv::gpu::DeviceInfo devInfo = GET_PARAM(0);
    cv::gpu::setDevice(devInfo.deviceID());

    std::string inputFile = perf::TestBase::getDataPath(std::string("gpu/video/") + GET_PARAM(1));
    int cn = GET_PARAM(2);

    cv::VideoCapture cap(inputFile);
    ASSERT_TRUE(cap.isOpened());

    cv::Mat frame;

    cv::gpu::GpuMat d_frame;
    cv::gpu::MOG2_GPU mog2;
    cv::gpu::GpuMat foreground;

    cap >> frame;
    ASSERT_FALSE(frame.empty());

    if (cn != 3)
    {
        cv::Mat temp;
        if (cn == 1)
            cv::cvtColor(frame, temp, cv::COLOR_BGR2GRAY);
        else
            cv::cvtColor(frame, temp, cv::COLOR_BGR2BGRA);
        cv::swap(temp, frame);
    }

    d_frame.upload(frame);

    mog2(d_frame, foreground);

    for (int i = 0; i < 10; ++i)
    {
        cap >> frame;
        ASSERT_FALSE(frame.empty());

        if (cn != 3)
        {
            cv::Mat temp;
            if (cn == 1)
                cv::cvtColor(frame, temp, cv::COLOR_BGR2GRAY);
            else
                cv::cvtColor(frame, temp, cv::COLOR_BGR2BGRA);
            cv::swap(temp, frame);
        }

        d_frame.upload(frame);

        startTimer(); next();
        mog2(d_frame, foreground);
        stopTimer();
    }
}

INSTANTIATE_TEST_CASE_P(Video, MOG2_update, testing::Combine(
    ALL_DEVICES,
    testing::Values(std::string("768x576.avi"), std::string("1920x1080.avi")),
    testing::Values(Channels(1), Channels(3), Channels(4))));

GPU_PERF_TEST(MOG2_getBackgroundImage, cv::gpu::DeviceInfo, std::string, Channels)
{
    cv::gpu::DeviceInfo devInfo = GET_PARAM(0);
    cv::gpu::setDevice(devInfo.deviceID());

    std::string inputFile = perf::TestBase::getDataPath(std::string("gpu/video/") + GET_PARAM(1));
    int cn = GET_PARAM(2);

    cv::VideoCapture cap(inputFile);
    ASSERT_TRUE(cap.isOpened());

    cv::Mat frame;

    cv::gpu::GpuMat d_frame;
    cv::gpu::MOG2_GPU mog2;
    cv::gpu::GpuMat foreground;

    for (int i = 0; i < 10; ++i)
    {
        cap >> frame;
        ASSERT_FALSE(frame.empty());

        if (cn != 3)
        {
            cv::Mat temp;
            if (cn == 1)
                cv::cvtColor(frame, temp, cv::COLOR_BGR2GRAY);
            else
                cv::cvtColor(frame, temp, cv::COLOR_BGR2BGRA);
            cv::swap(temp, frame);
        }

        d_frame.upload(frame);

        mog2(d_frame, foreground);
    }

    cv::gpu::GpuMat background;
    mog2.getBackgroundImage(background);

    TEST_CYCLE()
    {
        mog2.getBackgroundImage(background);
    }
}

INSTANTIATE_TEST_CASE_P(Video, MOG2_getBackgroundImage, testing::Combine(
    ALL_DEVICES,
    testing::Values(std::string("768x576.avi"), std::string("1920x1080.avi")),
    testing::Values(Channels(1), Channels(3), Channels(4))));

//////////////////////////////////////////////////////
// VIBE

GPU_PERF_TEST(VIBE, cv::gpu::DeviceInfo, std::string, Channels)
{
    cv::gpu::DeviceInfo devInfo = GET_PARAM(0);
    cv::gpu::setDevice(devInfo.deviceID());

    std::string inputFile = perf::TestBase::getDataPath(std::string("gpu/video/") + GET_PARAM(1));
    int cn = GET_PARAM(2);

    cv::VideoCapture cap(inputFile);
    ASSERT_TRUE(cap.isOpened());

    cv::Mat frame;
    cap >> frame;
    ASSERT_FALSE(frame.empty());

    if (cn != 3)
    {
        cv::Mat temp;
        if (cn == 1)
            cv::cvtColor(frame, temp, cv::COLOR_BGR2GRAY);
        else
            cv::cvtColor(frame, temp, cv::COLOR_BGR2BGRA);
        cv::swap(temp, frame);
    }

    cv::gpu::GpuMat d_frame(frame);
    cv::gpu::VIBE_GPU vibe;
    cv::gpu::GpuMat foreground;

    vibe(d_frame, foreground);

    for (int i = 0; i < 10; ++i)
    {
        cap >> frame;
        ASSERT_FALSE(frame.empty());

        if (cn != 3)
        {
            cv::Mat temp;
            if (cn == 1)
                cv::cvtColor(frame, temp, cv::COLOR_BGR2GRAY);
            else
                cv::cvtColor(frame, temp, cv::COLOR_BGR2BGRA);
            cv::swap(temp, frame);
        }

        d_frame.upload(frame);

        startTimer(); next();
        vibe(d_frame, foreground);
        stopTimer();
    }
}

INSTANTIATE_TEST_CASE_P(Video, VIBE, testing::Combine(
    ALL_DEVICES,
    testing::Values(std::string("768x576.avi"), std::string("1920x1080.avi")),
    testing::Values(Channels(1), Channels(3), Channels(4))));

//////////////////////////////////////////////////////
// VideoWriter

#ifdef WIN32

GPU_PERF_TEST(VideoWriter, cv::gpu::DeviceInfo, std::string)
{
    const double FPS = 25.0;

    cv::gpu::DeviceInfo devInfo = GET_PARAM(0);
    cv::gpu::setDevice(devInfo.deviceID());

    std::string inputFile = perf::TestBase::getDataPath(std::string("gpu/video/") + GET_PARAM(1));
    std::string outputFile = cv::tempfile(".avi");

    cv::VideoCapture reader(inputFile);
    ASSERT_TRUE( reader.isOpened() );

    cv::gpu::VideoWriter_GPU d_writer;

    cv::Mat frame;
    cv::gpu::GpuMat d_frame;

    declare.time(10);

    for (int i = 0; i < 10; ++i)
    {
        reader >> frame;
        ASSERT_FALSE(frame.empty());

        d_frame.upload(frame);

        if (!d_writer.isOpened())
            d_writer.open(outputFile, frame.size(), FPS);

        startTimer(); next();
        d_writer.write(d_frame);
        stopTimer();
    }
}

INSTANTIATE_TEST_CASE_P(Video, VideoWriter, testing::Combine(
    ALL_DEVICES,
    testing::Values(std::string("768x576.avi"), std::string("1920x1080.avi"))));

#endif // WIN32

//////////////////////////////////////////////////////
// VideoReader

GPU_PERF_TEST(VideoReader, cv::gpu::DeviceInfo, std::string)
{
    cv::gpu::DeviceInfo devInfo = GET_PARAM(0);
    cv::gpu::setDevice(devInfo.deviceID());

    std::string inputFile = perf::TestBase::getDataPath(std::string("gpu/video/") + GET_PARAM(1));

    cv::gpu::VideoReader_GPU reader(inputFile);
    ASSERT_TRUE( reader.isOpened() );

    cv::gpu::GpuMat frame;

    reader.read(frame);

    declare.time(20);

    TEST_CYCLE_N(10)
    {
        reader.read(frame);
    }
}

INSTANTIATE_TEST_CASE_P(Video, VideoReader, testing::Combine(
    ALL_DEVICES,
    testing::Values(std::string("768x576.avi"), std::string("1920x1080.avi"))));

#endif
