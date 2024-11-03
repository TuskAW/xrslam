#ifndef XRSLAM_PC_EUROC_DATASET_READER_H
#define XRSLAM_PC_EUROC_DATASET_READER_H

#include <dataset_reader.h>
#include <deque>
#include <cstring>

class EurocDatasetReader : public DatasetReader {
  public:
    EurocDatasetReader(const std::string &filename, void *yaml_config);
    NextDataType next() override;

    void get_image_resolution(int &width, int &height) override;
    std::pair<double, cv::Mat> read_image() override;
    std::pair<double, XRSLAMGyroscope> read_gyroscope() override;
    std::pair<double, XRSLAMAcceleration> read_accelerometer() override;

  private:
    std::shared_ptr<xrslam::extra::YamlConfig> config;
    std::deque<std::pair<double, NextDataType>> all_data;
    std::deque<std::pair<double, XRSLAMGyroscope>> gyroscope_data;
    std::deque<std::pair<double, XRSLAMAcceleration>> accelerometer_data;
    std::deque<std::pair<double, std::string>> image_data;
    int image_width;
    int image_height;
};

struct CameraCsv {
    struct CameraData {
        double t;
        std::string filename;
    };

    std::vector<CameraData> items;

    void load(const std::string &filename) {
        items.clear();
        if (FILE *csv = fopen(filename.c_str(), "r")) {
            char header_line[2048];
            #ifdef _WIN32
                int ret = fscanf(csv, "%2047[^\r]\r\n", header_line);
            #else
                int ret = fscanf(csv, "%2047[^\n]\n", header_line);
            #endif
            char filename_buffer[2048];
            CameraData item;
            while (!feof(csv)) {
                memset(filename_buffer, 0, 2048);
            #ifdef _WIN32
                if (fscanf(csv, "%lf,%2047[^\r]\r\n", &item.t,
                           filename_buffer) != 2) {
            #else
                if (fscanf(csv, "%lf,%2047[^\n]\n", &item.t,
                           filename_buffer) != 2) {
            #endif
                    break;
                }
                item.t *= 1e-9;
                item.filename = std::string(filename_buffer);
                items.emplace_back(std::move(item));
            }
            fclose(csv);
        }
    }

    void save(const std::string &filename) const {
        if (FILE *csv = fopen(filename.c_str(), "w")) {
            fputs("#t[ns],filename[string]\n", csv);
            for (auto item : items) {
                fprintf(csv, "%ld,%s\n", int64_t(item.t * 1e9),
                        item.filename.c_str());
            }
            fclose(csv);
        }
    }
};

struct ImuCsv {
    struct ImuData {
        double t;
        struct {
            double x;
            double y;
            double z;
        } w;
        struct {
            double x;
            double y;
            double z;
        } a;
    };

    std::vector<ImuData> items;

    void load(const std::string &filename) {
        items.clear();
        if (FILE *csv = fopen(filename.c_str(), "r")) {
            char header_line[2048];
            #ifdef _WIN32
                int ret = fscanf(csv, "%2047[^\r]\r\n", header_line);
            #else
                int ret = fscanf(csv, "%2047[^\n]\n", header_line);
            #endif
            ImuData item;
            #ifdef _WIN32
                while (!feof(csv) &&
                       fscanf(csv, "%lf,%lf,%lf,%lf,%lf,%lf,%lf\r\n", &item.t,
                              &item.w.x, &item.w.y, &item.w.z, &item.a.x,
                              &item.a.y, &item.a.z) == 7) {
            #else
                while (!feof(csv) &&
                       fscanf(csv, "%lf,%lf,%lf,%lf,%lf,%lf,%lf\n", &item.t,
                              &item.w.x, &item.w.y, &item.w.z, &item.a.x,
                              &item.a.y, &item.a.z) == 7) {
            #endif
                item.t *= 1e-9;
                items.emplace_back(std::move(item));
            }
            fclose(csv);
        }
    }

    void save(const std::string &filename) const {
        if (FILE *csv = fopen(filename.c_str(), "w")) {
            fputs("#t[ns],w.x[rad/s:double],w.y[rad/s:double],w.z[rad/"
                  "s:double],a.x[m/s^2:double],a.y[m/s^2:double],a.z[m/"
                  "s^2:double]\n",
                  csv);
            for (auto item : items) {
                fprintf(csv, "%ld,%.9e,%.9e,%.9e,%.9e,%.9e,%.9e\n",
                        int64_t(item.t * 1e9), item.w.x, item.w.y, item.w.z,
                        item.a.x, item.a.y, item.a.z);
            }
            fclose(csv);
        }
    }
};

#endif
