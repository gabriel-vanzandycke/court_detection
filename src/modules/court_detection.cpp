#include <string>
#include <iostream> // TODO: remove

#include <opencv2/ximgproc.hpp>

#include "court_detection.hpp"

CourtDetection::CourtDetection(std::string court_type){
    // build list of operations
    // build court model
}


cv::Mat CourtDetection::operator()(cv::Mat& image) {
    // skeletonize
    cv::Mat skeleton = image.clone();
    cv::ximgproc::thinning(image, skeleton);

    // remove small connected components
    cv::Mat labeled_image, stats, centroids, mask;
    int n_labels = connectedComponentsWithStats(skeleton, labeled_image, stats, centroids);
    for (int i = 1; i < n_labels; ++i) // skip background component
    {
        if (stats.at<int>(i, cv::CC_STAT_AREA) < 50)
        {
            cv::compare(labeled_image, i, mask, cv::CMP_EQ);
            skeleton.setTo(0, mask);
        }
    }

    // find segments
    std::vector<cv::Vec4i> segments;
    float distance_step = .1;
    float angle_step = .1;
    int threshold = 10;
    int min_line_length = 100;
    int max_line_gap = 100;
    cv::HoughLinesP(skeleton, segments, distance_step, angle_step*CV_PI/180, threshold, min_line_length, max_line_gap);


    // draw lines
    cv::Mat output;
    cv::cvtColor(image, output, cv::COLOR_GRAY2RGB);
    for( size_t i = 0; i < segments.size(); i++ )
    {
        cv::Vec4i l = segments[i];
        cv::line( output, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0,0,255), 3);

    }

    return output;
}
