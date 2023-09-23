#include <iostream>
#include <vector>
#include <cmath>
#include <fftw3.h>
#include <mpg123.h>
#include "matplotlib-cpp/matplotlibcpp.h"

// Function to calculate the FFT of a given audio buffer
std::vector<double> calculateFFT(const std::vector<double>& audioData) {
    int dataSize = audioData.size();
    std::vector<double> fftResult(dataSize);

    fftw_complex* in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * dataSize);
    fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * dataSize);

    fftw_plan plan = fftw_plan_dft_1d(dataSize, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    for (int i = 0; i < dataSize; ++i) {
        in[i][0] = audioData[i];
        in[i][1] = 0.0;
    }

    fftw_execute(plan);

    for (int i = 0; i < dataSize; ++i) {
        fftResult[i] = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
    }

    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);

    return fftResult;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_mp3_file>" << std::endl;
        return 1;
    }

    // Initialize the mpg123 library
    mpg123_init();
    mpg123_handle* mh = mpg123_new(NULL, NULL);
    if (mpg123_open(mh, argv[1]) != MPG123_OK) {
        std::cerr << "Failed to open MP3 file: " << argv[1] << std::endl;
        return 1;
    }

    // Read and decode the MP3 file
    long rate;
    int channels, encoding;
    if (mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK) {
        std::cerr << "Failed to get MP3 format information." << std::endl;
        return 1;
    }

    std::vector<double> audioData;
    mpg123_format_none(mh);
    mpg123_format(mh, rate, channels, encoding);
    size_t bufferSize = mpg123_outblock(mh);
    unsigned char* buffer = new unsigned char[bufferSize];
    size_t bytesRead;

    while (mpg123_read(mh, buffer, bufferSize, &bytesRead) == MPG123_OK) {
        for (size_t i = 0; i < bytesRead; ++i) {
            audioData.push_back(static_cast<double>(buffer[i]) / 32767.0); // Normalize to [-1, 1]
        }
    }

    delete[] buffer;
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();

    // Calculate FFT on the audio data
    std::vector<double> fftResult = calculateFFT(audioData);
	// for (std::vector<double>::iterator it = fftResult.begin(); it != fftResult.end(); it++) {
	// 	std::cout << *it << std::endl;
	// }
    // Now you can analyze the FFT result or perform further processing
	matplotlibcpp::plot(fftResult);
	matplotlibcpp::show();

    return 0;
}
