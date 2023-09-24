#include "main.hpp"

double playback_time = 5;
bool live_playback = true;
std::vector<double> sin_wave_frequency_table = {220, 330, 275};
std::vector<double> sin_wave_amplitude_table = {1, 0.5, 0.75};
std::vector<double> sin_wave_position_table = {0, 0, 0};
std::vector<double> square_wave_frequency_table = {440};
std::vector<double> square_wave_amplitude_table = {0.05};
std::vector<double> square_wave_position_table = {0, 0};
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

double SawWave(double argument) {
	return ((argument * 2) - 1);
}

void AdvanceWavePosition(double* wave_position, double frequency, int sample_rate) {
	double old_wave_position = *wave_position;
	*wave_position = old_wave_position + (frequency / sample_rate);
	if (*wave_position > 1) {
		*wave_position = *wave_position - floor(*wave_position);
	}
}

std::vector<int8_t> MakeStaticSounds() {
	std::vector<int8_t> sound_data_vector;
	uint64_t total_sample_count = SAMPLE_RATE * playback_time;
	int16_t sound_sample;
	double combined_wave_amplitude;
	double wave_aplitude_sum;
	for (uint64_t sample_iterator = 0; sample_iterator < total_sample_count; sample_iterator++) {
		combined_wave_amplitude = 0;
		wave_aplitude_sum = 0;
		for (auto sin_table_iterator = 0; sin_table_iterator < sin_wave_position_table.size(); sin_table_iterator++) {
			wave_aplitude_sum += sin_wave_amplitude_table[sin_table_iterator];
			combined_wave_amplitude += sin_wave_amplitude_table[sin_table_iterator] * SinWave(sin_wave_position_table[sin_table_iterator]);
			AdvanceWavePosition(&sin_wave_position_table[sin_table_iterator], sin_wave_frequency_table[sin_table_iterator], SAMPLE_RATE);
		}
		for (auto square_table_iterator = 0; square_table_iterator < square_wave_position_table.size(); square_table_iterator++) {
			wave_aplitude_sum += square_wave_amplitude_table[square_table_iterator];
			combined_wave_amplitude += square_wave_amplitude_table[square_table_iterator] * SquareWave(square_wave_position_table[square_table_iterator]);
			AdvanceWavePosition(&square_wave_position_table[square_table_iterator], square_wave_frequency_table[square_table_iterator], SAMPLE_RATE);
		}
		sound_sample = 32767 * (combined_wave_amplitude / wave_aplitude_sum);
		uint8_t sound_sample_0, sound_sample_1;
		sound_sample_0 = static_cast<uint8_t>(sound_sample); // Least significant byte (LSB)
		sound_sample_1 = static_cast<uint8_t>(sound_sample >> 8); // Most significant byte (MSB)
		sound_data_vector.push_back(sound_sample_0);
		sound_data_vector.push_back(sound_sample_1);
	}
	return sound_data_vector;
}

int main(int argc, char** argv) {
	if (live_playback) {
		PaError err;
		err = Pa_Initialize();
		if (err != paNoError) {
			fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
			return -1;
		}
		// Specify PulseAudio as the host API
		PaStream *stream;
		err = Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, 44100, 256, NULL, NULL);
		if (err != paNoError) {
			fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
			return -1;
		}
		err = Pa_StartStream(stream);
		if (err != paNoError) {
			fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
			return -1;
		}
		// Generate and play a sine wave (replace with your audio data)
		std::vector<float> audioData(256, 0.0f);
		float phase = 0.0f;
		float frequency = 440.0f;  // A4 (440 Hz)
		while (1) {
			for (int i = 0; i < 256; i++) {
				audioData[i] = 0.5f * sin(2.0f * M_PI * frequency * phase);
				phase += 1.0f / 44100.0f;
			}
			err = Pa_WriteStream(stream, audioData.data(), 256);
			if (err != paNoError) {
				fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
				return -1;
			}
		}
		// Cleanup
		Pa_StopStream(stream);
		Pa_CloseStream(stream);
		Pa_Terminate();
		return 0;
	}
	else {
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
}
