#include <opencv2/opencv.hpp>
#include <algorithm>
#include <vector>
#include <string> // Para std::to_string

void plotHeatmap(const std::vector<std::vector<float>>& M, int factor, const std::vector<std::vector<int>>& Z = {}, bool showLabels = false) {
    int rows = M.size();
    int cols = M[0].size();
    cv::Mat matM(rows, cols, CV_32F);
    
    for(int i = 0; i < rows; ++i) {
        for(int j = 0; j < cols; ++j) {
            matM.at<float>(i,j) = M[i][j];
        }
    }
    
    cv::Mat matZ;
    if (!Z.empty()) {
        matZ = cv::Mat(rows, cols, CV_32S);
        for(int i = 0; i < rows; ++i) {
            for(int j = 0; j < cols; ++j) {
                matZ.at<int>(i,j) = Z[i][j];
            }
        }
    }
    
    cv::Mat M_big;
    cv::resize(matM, M_big, cv::Size(), factor, factor, cv::INTER_CUBIC);

    cv::Mat Mnorm, heatmap;
    cv::normalize(M_big, Mnorm, 0, 255, cv::NORM_MINMAX, CV_8U);
    cv::applyColorMap(Mnorm, heatmap, cv::COLORMAP_VIRIDIS);

    if (!matZ.empty()) {
        cv::Mat heatmapRGBA;
        cv::cvtColor(heatmap, heatmapRGBA, cv::COLOR_BGR2BGRA);
        
        cv::Mat Z_big;
        cv::resize(matZ, Z_big, cv::Size(), factor, factor, cv::INTER_NEAREST);

        for (int i = 0; i < Z_big.rows; ++i) {
            for (int j = 0; j < Z_big.cols; ++j) {
                int currentZone = Z_big.at<int>(i, j);
                bool isBorder = false;

                if (j + 1 < Z_big.cols) {
                    if (currentZone != Z_big.at<int>(i, j + 1)) {
                        isBorder = true;
                    }
                }
                
                if (i + 1 < Z_big.rows) {
                    if (currentZone != Z_big.at<int>(i + 1, j)) {
                        isBorder = true;
                    }
                }

                if (isBorder) {
                    heatmapRGBA.at<cv::Vec4b>(i, j) = cv::Vec4b(0, 0, 0, 255);
                }
            }
        }
        
        // La flag para mostrar los id de cada zona
        
        if (showLabels) {
            double fontScale = std::min(0.8, std::max(0.2, (double)factor / 40.0));
            int thickness = (fontScale > 0.4) ? 2 : 1;

            // Iteramos sobre la matriz de zonas ORIGINAL
            for(int i = 0; i < matZ.rows; ++i) {
                for(int j = 0; j < matZ.cols; ++j) {
                    
                   
                    int zone_id = matZ.at<int>(i,j);
                    std::string zone_text = std::to_string(zone_id);

                    int draw_x = static_cast<int>((j + 0.5) * factor);
                    int draw_y = static_cast<int>((i + 0.5) * factor);

                    cv::Size textSize = cv::getTextSize(zone_text, cv::FONT_HERSHEY_SIMPLEX, fontScale, thickness, 0);
                    cv::Point textOrg(draw_x - textSize.width / 2, draw_y + textSize.height / 2);

                    cv::putText(heatmapRGBA, 
                                zone_text, 
                                textOrg, 
                                cv::FONT_HERSHEY_SIMPLEX, 
                                fontScale,
                                cv::Scalar(0, 0, 0, 255),
                                thickness + 1, 
                                cv::LINE_AA);
                    cv::putText(heatmapRGBA, 
                                zone_text, 
                                textOrg, 
                                cv::FONT_HERSHEY_SIMPLEX, 
                                fontScale,
                                cv::Scalar(255, 255, 255, 255), 
                                thickness,
                                cv::LINE_AA);
                }
            }
        }
        
        int borderExtension = 2;
        cv::Mat expandedHeatmap;
        cv::copyMakeBorder(heatmapRGBA, expandedHeatmap, borderExtension, borderExtension, 
                          borderExtension, borderExtension, cv::BORDER_CONSTANT, cv::Scalar(255,255,255,255));

        cv::cvtColor(expandedHeatmap, heatmap, cv::COLOR_BGRA2BGR);
    }

    std::string windowTitle = !matZ.empty() ? "Mapa de Calor con Zonas" : "Mapa de Calor";
    cv::imshow(windowTitle, heatmap);
    cv::waitKey(0);
}