#include <iostream>
#include <iomanip>
#include <chrono>
#include <fstream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#ifdef __WIN32__
#include <winsock.h>
#else
#include <netinet/in.h>
#endif
#include <vector>
#include "AudioDriver.h"
#include "SoundSorce.h"
#include "FMWaveGenerator.h"

int main(int, char**)
{
	SDL_Init(SDL_INIT_EVERYTHING);

	FMInstrument inst
			{ 5, true, {
			  { 1., 0.1, 0.9, 0.000001, 1.0, 0.,1.,0.7},
			  { 1., 0.1, 0.9, 0.000001, 1.0, 0.,1.,0.2 },
			  { 1., 0.1, 0.9, 0.000001, 1.0, 0.,0.5,0.3},
			  { 1., 0.1, 0.9, 0.000001, 1.0, 0.,0.25,0.5} } };

	AudioDriver drv;
	drv.unpause();

	auto src = std::make_shared<SoundSource>();
	auto wg = std::make_shared<FMWaveGenerator>();
	auto eg = std::make_shared<ConstantEnvelopeGenerator>(1.);

	wg->setup(inst);
	src->set_envelope_generator(eg);
	src->set_wave_generator(wg);

	drv.set_sound_source(src);
	drv.set_master_volume(4000);


	auto win = std::shared_ptr<SDL_Window>(
			SDL_CreateWindow("", 50, 50, 320, 240, SDL_WINDOW_SHOWN),
			SDL_DestroyWindow);

	static const double s_freq_table[]={8.17 , 8.66 , 9.18 , 9.72 , 10.30 , 10.91 , 11.56 , 12.25 , 12.98 , 13.75 , 14.57 , 15.43,
										16.35 , 17.32 ,  18.35 , 19.45 , 20.60 , 21.83 , 23.12 , 24.50 , 25.96 , 27.5 , 29.13,30.87,
										32.70 , 34.65 , 36.71 , 38.89 , 41.20 , 43.65 , 46.25 , 48.99 , 51.91 , 55. , 58.27 , 61.73,
										65.41 , 69.29 , 73.42 , 77.78 , 82.41 , 87.31 , 92.49 , 97.99 , 103.83 , 110. , 116.54 , 123.47,
										130.81 , 138.59 , 146.83 , 155.56 , 164.81 , 174.61 , 184.99 , 195.99 , 207.65 , 220. , 233.08 , 246.94,
										261.62 , 277.18 , 293.66 , 311.13 , 329.63 , 349.23 , 369.99 , 391.99 , 415.30 , 440. , 466.16 , 493.88,
										523.25 , 554.36 , 587.33 , 622.25 , 659.25 , 698.46 , 739.99 , 783.99 , 830.61 , 880. , 932.33 , 987.77,
										1046.50 , 1108.73 ,  1174.66 , 1244.51 , 1318.51 , 1396.91 , 1479.97 , 1567.98 , 1661.22 , 1760. , 1864.65,1975.53,
										2093. , 2217.46 , 2349.32 , 2489.02 , 2637.02 , 2793.83 , 2959.95 , 3135.96 , 3322.44 , 3520. , 3729.31 , 3951.07,
										4186. , 4434.92 , 4698.64 , 4978.03 , 5274.04 , 5587.65 , 5919.91 , 6271.93 , 6644.87 , 7040. , 7458.62 , 7902.13,
										8372.02 , 8869.84 , 9397.27 , 9956.06 , 10546.08 , 11175.3 , 11839.82 , 12543.85};
	std::vector<std::vector<char>> track;

		std::fstream file;

		file.open("badapple2.mid", std::ios::in | std::ios::binary);

		int chunk_type;
		int MThd_length;
		short MThd_format;
		short MThd_tracks;
		short MThd_division;
		int tempo = 500000;
		int old_event_type = 0;
		int nn,dd,cc = 0x18,bb = 0x08;
		int l,l_old,s;



		file.read(reinterpret_cast<char*>(&chunk_type), sizeof(chunk_type));
		if (chunk_type == (
				'M'*0x00000001 +
				'T'*0x00000100 +
				'h'*0x00010000 +
				'd'*0x01000000))
			std::cout << "Загаловок" << std::endl;

		file.read(reinterpret_cast<char*>(&MThd_length), sizeof(MThd_length));
		MThd_length = ntohl(MThd_length);
		std::cout << "MThd length: " << MThd_length << std::endl;

		file.read(reinterpret_cast<char*>(&MThd_format), sizeof(MThd_format));
		MThd_format = ntohs(MThd_format);
		std::cout << "MThd format: " << MThd_format << std::endl;

		file.read(reinterpret_cast<char*>(&MThd_tracks), sizeof(MThd_tracks));
		MThd_tracks = ntohs(MThd_tracks);
		std::cout << "MThd tracks: " << MThd_tracks << std::endl;

		file.read(reinterpret_cast<char*>(&MThd_division), sizeof(MThd_division));
		MThd_division = ntohs(MThd_division);
		std::cout << "MThd division: " << MThd_division << std::endl;
		if (MThd_division & 0x8000) {
			std::cout << "Единица измерения - SMPTE" << std::endl;
		} else {
			std::cout << "Единица измерения - TPQN" << std::endl;
		}

		track.resize(MThd_tracks);


		for (int i = 0; i < MThd_tracks; i++) {
			file.read(reinterpret_cast<char*>(&chunk_type), sizeof(chunk_type));
			if (chunk_type == (
					'M'*0x00000001 +
					'T'*0x00000100 +
					'r'*0x00010000 +
					'k'*0x01000000)) {
				std::cout << "дорожка 1" << i << std::endl;
			} else {
				std::cout << "не та дорожка" << std::endl;
				break;
			}
			int trk_length;
			file.read(reinterpret_cast<char*>(&trk_length), sizeof(int));
			trk_length = ntohl(trk_length);
			std::cout << "  Длина: " << trk_length << std::endl;
			track[i].resize(trk_length);
			file.read(&track[i][0], trk_length);
		}

		size_t trk = 3;
		std::cout << " " << std::endl;

		using clk = std::chrono::steady_clock;
		auto pop = std::chrono::microseconds(tempo)/(cc*bb)/2;
		auto tp_now = clk::now();
		auto tp_next = tp_now;

		size_t p = 0;
		while (p < track[trk].size()) {
			int delta = 0;
			int delta_v;
			do {
			  delta_v = track[trk][p++];
			  delta *= 128;
			  delta += delta_v & 0x7f;
			} while (delta_v & 0x80);
			tp_next = tp_next+pop*delta;
			do{
				tp_now = clk::now();
			}while(tp_now<tp_next);


			std::cout << std::setw(10) << delta << ":---> ";

			unsigned int event_type = 0x000000ff & track[trk][p++];

			if (event_type < 0x80) {
			  event_type = old_event_type;
			  p--;
			} else {
			  old_event_type = event_type;
			}

			std::cout << "байт события " << event_type << std::endl;

			switch(event_type) {
			case 0x80 ... 0x8f:{
				l = track[trk][p++];
				s = int(track[trk][p++]);
				if(l==l_old) wg->key_off();
				std::cout << "Выключить ноту " << l << " c громкостью "<<  s <<std::endl;
			}
				break;
			case 0x90 ... 0x9f:{
				l = track[trk][p++];
				s = int(track[trk][p++]);
				l_old=l;
				eg->set_level(s/127.0);
				wg->set_freq(s_freq_table[l]);
				wg->key_on();
				std::cout << "Включить ноту " << l << " c громкостью "<<  s <<std::endl;
			}
				break;
			case 0xa0 ... 0xaf:
				std::cout << " Это метасобытие3 - пропуск" << std::endl;
				break;
			case 0xb0 ... 0xbf:
				int num_par, par;
				num_par = int(track[trk][p++]);
				par = int(track[trk][p++]);
				std::cout << num_par<<" " <<par<<std::endl;
				std::cout << " Control Change" << std::endl;
				break;
			case 0xc0 ... 0xcf:
				std::cout << " Program Change " << std::endl;

				p++;
				break;
			case 0xd0 ... 0xdf:
				std::cout << "Aftertouch" ;
				std::cout << " Его параметр - " << int(track[trk][p++])<< std::endl;
				break;
			case 0xe0 ... 0xef:{
				int l1,m1;
				l1=int(track[trk][p++]);
				m1=int(track[trk][p++]);
				int bend = m1*128+l1;
				bend-=0x2000;
				std::cout << " Pitch Wheel Change" <<bend<< std::endl;
				double f1,f2,fb;
				if(bend>0){
					f1 = s_freq_table[l];
					f2 = s_freq_table[l+1];
					fb = f1 + (bend / 8192.0) * (f2-f1);
				}else if(bend<0){
					f1 = s_freq_table[l-1];
					f2 = s_freq_table[l];
					fb = f2 + (bend / 8192.0) * (f2-f1);
				}else{
					fb = s_freq_table[l];
				}
				wg->set_freq(fb);
			}

				break;
			case 0xf0:{
				int q=0;
				while(q!=0xf7){
					q = int(track[trk][p++]& 0xff);
				}
				p++;
				std::cout << " Это метасобытие8" << std::endl;
			}
				break;
			case 0xf1:
				std::cout << " Это метасобытие9" << std::endl;
				break;
			case 0xf2:
				std::cout << " Это метасобытие10" << std::endl;
				break;
			case 0xf3:
				std::cout << " Это метасобытие12" << std::endl;
				break;
			case 0xf4:
				std::cout << " Это метасобытие13" << std::endl;
				break;
			case 0xf5:
				std::cout << " Это метасобытие14" << std::endl;
				break;
			case 0xf6:
				std::cout << " Это метасобытие15" << std::endl;
				break;
			case 0xf7:
				std::cout << " Это метасобытие16" << std::endl;
				break;
			case 0xf8:
				std::cout << " Это метасобытие17" << std::endl;
				break;
			case 0xf9:
				std::cout << " Это метасобытие18" << std::endl;
				break;
			case 0xfa:
				std::cout << " Это метасобытие19" << std::endl;
				break;
			case 0xfb:
				std::cout << " Это метасобытие20" << std::endl;
				break;
			case 0xfc:
				std::cout << " Это метасобытие21" << std::endl;
				break;
			case 0xfd:
				std::cout << " Это метасобытие22" << std::endl;
				break;
			case 0xfe:
				std::cout << " Это метасобытие23" << std::endl;
				break;
			case 0xff:
			{
				int meta_type = int(track[trk][p++]);
				switch(meta_type) {
					case 0x01:{
						unsigned len = unsigned(track[trk][p++]);
						std::string str;
						while (len--) str += char(track[trk][p++]);
						std::cout << "Здесь написано: " << str << std::endl;
					}
						break;
					case 0x02:
						std::cout << "АА" << std::endl;
						break;
					case 0x03:
					  {
						unsigned len = unsigned(track[trk][p++]);
						std::string str;
						while (len--) str += char(track[trk][p++]);
						std::cout << "Название дорожки: " << str << std::endl;
					  }
					  break;
					case 0x04:
						std::cout << "ААА" << std::endl;
						break;
					case 0x06:{
						unsigned len = unsigned(track[trk][p++]);
						std::string str;
						while (len--) str += char(track[trk][p++]);
						std::cout << "Название: " << str << std::endl;
					}
						break;
					case 0x2f:
						p++;
						std::cout << "Конец трека" << std::endl;
						return 0;
						break;
					case 0x51:
						p++;
						tempo += 0x10000 * int(track[trk][p++]);
						tempo += 0x100   * int(track[trk][p++]);
						tempo +=           int(track[trk][p++]);
						std::cout << "Задание темпа" << std::endl;
						pop = std::chrono::microseconds(tempo)/(cc*bb)/2;
						break;
					case 0x58:
						p++;
						nn = int(track[trk][p++]);
						dd = int(track[trk][p++]);
						cc = int(track[trk][p++]);
						bb = int(track[trk][p++]);
						std::cout << nn<<" " <<dd<< " " <<cc<< " " << bb <<std::endl;
						break;
					default:
					  std::cout << "нету меты " << meta_type << std::endl;
					}
			}
				break;
			default:
				std::cout << "нету события" << std::endl;
				return 1;
			}
		}


		file.close();


}


