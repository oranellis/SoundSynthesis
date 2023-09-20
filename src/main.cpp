#include "main.hpp"

double playback_time = 1;
double frequency_hz = 220;
enum playback_type {
	live,
	wav
};

#pragma pack(push, 1) // Ensure that the struct is packed tightly
typedef struct WavHeaderStruct {
    // RIFF Chunk
    char riff_chunk_id[4] = {'R','I','F','F'};      // "RIFF"
    uint32_t total_file_size;   // Total file size - 8 bytes
    char wave_format[4] = {'W', 'A', 'V', 'E'};        // "WAVE"
    // Format Subchunk
    char fmt_chunk_id[4] = {'f','m','t',' '};       // "fmt "
    uint32_t fmt_chunk_size = 16;    // Size of the format chunk (16 for PCM)
    uint16_t audio_format = 1;      // Audio format (1 for PCM)
    uint16_t num_channels = 1;      // Number of channels (1 for mono, 2 for stereo)
    uint32_t sample_rate = SAMPLE_RATE;       // Sample rate (e.g., 44100)
    uint32_t byte_rate = SAMPLE_RATE * 2;         // Byte rate (sample_rate * num_channels * bits_per_sample / 8)
    uint16_t block_align = 2;       // Block align (num_channels * bits_per_sample / 8)
    uint16_t bits_per_sample = 16;   // Bits per sample (8, 16, etc.)
    // Data Subchunk
    char data_chunk_id[4] = {'d','a','t','a'};      // "data"
    uint32_t data_chunk_size;   // Size of the data chunk (audio data length in bytes)
} WavHeaderStruct;
#pragma pack(pop) // Restore default packing

double SinWave(double argument) {
	return sin(argument*TWO_PI);
}

double SquareWave(double argument) {
	return (argument-floor(argument) < 0.5 ? -1 : 1);
}

double IncrementWavePosition(double wave_position, double frequency, int sample_rate);

// double WaveProfile1(double time) {
// 	if (time < 1) {

// 	}
// 	else if (time < 3) {

// 	}
// }

std::vector<int8_t> MakeStaticSounds() {
	std::vector<int8_t> sound_data_vector;
	uint64_t total_sample_count = SAMPLE_RATE * playback_time;
	int16_t sound_sample;
	double sin_counter;
	double wave_argument;
	for (uint64_t sample_iterator = 0; sample_iterator < total_sample_count; sample_iterator++) {
		wave_argument = static_cast<double>(sample_iterator) * frequency_hz / SAMPLE_RATE;
		sound_sample = 32767 * (SquareWave(wave_argument));
		uint8_t sound_sample_0, sound_sample_1;
		sound_sample_0 = static_cast<uint8_t>(sound_sample); // Least significant byte (LSB)
		sound_sample_1 = static_cast<uint8_t>(sound_sample >> 8); // Most significant byte (MSB)
		sound_data_vector.push_back(sound_sample_0);
		sound_data_vector.push_back(sound_sample_1);
		sin_counter = ((static_cast<double>(sample_iterator * frequency_hz) / SAMPLE_RATE) * TWO_PI);
	}
	return sound_data_vector;
}

int main(int argc, char** argv) {
	int opt;
	bool option_l_present = false;
	while ((opt = getopt(argc, argv, "l")) != -1) {
		switch (opt) {
			case 'l':
				option_l_present = true;
				break;
			case '?':
				std::cerr << "Unknown option: -" << static_cast<char>(optopt) << std::endl;
				return 1;
			default:
				break;
		}
	}
	std::ofstream wav_output_file("sounds.wav", std::ios::out | std::ios::binary);
	if(!wav_output_file) {
		std::cout << "Cannot open file!" << std::endl;
		return 1;
	}
	std::vector<int8_t> sound_data;
	sound_data = MakeStaticSounds();
	WavHeaderStruct wav_header;
	wav_header.total_file_size = sizeof(WavHeaderStruct)+(sound_data.size()); // Sound_data contains bytes so size is size()
	wav_header.data_chunk_size = sound_data.size();
	wav_output_file.write((char*)&wav_header, sizeof(WavHeaderStruct));
	for (uint64_t it = 0; it < sound_data.size(); it++) {
		char insert = (char)sound_data[it];
		wav_output_file.write(&insert, 1); // need to write bytes not chars (which are two bytes), maybe compile into chars first
	}
	wav_output_file.close();
	return 0;
}
