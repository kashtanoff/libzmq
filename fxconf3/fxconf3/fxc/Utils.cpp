#pragma once

#include <windows.h>
#include <sstream>
#include <iomanip>
#include "debug/Debug.h"
#include "Convert.h"
#include "fxc.h"

#include <cstdio>
//#include <string>


namespace fxc {

namespace utils {

	// Реализует циклический массив-таймсерию в стиле mql4
	template <typename T>
	class CircularBuffer {

		public:

			CircularBuffer() {
				_buffer = nullptr;
				_size = 0;
				_index = 0;
			}
			
			~CircularBuffer() {
				delete []_buffer;
			}

			void alloc(int size) {
				if (size == _size) {
					return;
				}
				MARK_FUNC_IN
				if (_buffer == nullptr) {
					_buffer = new T[size];
					memset(_buffer, 0, sizeof(T) * size);
				}
				else {
#if DEBUG
					msg << "CircularBuffer second allocation!" << _size << " -> " << size << "\r\n" << msg_box;
#endif
					auto b = new T[size];
					/*
					if (size > _size - _index) {
						memcpy(b, _buffer + _index, _size - _index);
						memcpy(b + _size - _index, _buffer, size - _size + _index);
					}
					else {
						memcpy(b, _buffer + _index, size);
					}*/

					delete []_buffer;
					_buffer = b;
					memset(_buffer, 0, sizeof(T) * size);
				}
				_size  = size;
				_index = 0;
				MARK_FUNC_OUT
			}

			inline void add(T value) {
				_index = _index ? _index-1 : _size-1;
#if DEBUG
				if (_buffer == nullptr) {
					throw std::runtime_error("CircularBuffer add _buffer not init");
				}
				if (_index < 0 || _index >= _size) {
					throw std::out_of_range("CircularBuffer add index wrong");
				}
#endif
				_buffer[_index] = value;
			}
			
			inline void update(T value) {
#if DEBUG
				if (_buffer == nullptr) {
					throw std::runtime_error("CircularBuffer update _buffer not init");
				}
#endif
				_buffer[_index] = value;
			}

			inline void skip(int i) {
				i = i % _size;
				_index = i > _index ? 
					_index - i + _size : 
					_index - i;
#if DEBUG
				if (i < 0 || i >= _size) {
					throw std::out_of_range("CircularBuffer skip input index wrong");
				}
				if (_index < 0 || _index >= _size) {
					throw std::out_of_range("CircularBuffer skip index wrong");
				}
#endif

			}
			
			inline T& operator[](const int i) {
#if DEBUG
				if (_buffer == nullptr) {
					throw std::runtime_error("CircularBuffer [] _buffer not init");
				}
				if (i < 0 || i >= _size) {
					throw std::out_of_range("CircularBuffer input index wrong");
				}
				int ii = (_index + i) % _size;
				if (ii < 0 || ii >= _size) {
					throw std::out_of_range("CircularBuffer output index wrong");
				}
#endif
				return _buffer[(_index+i) % _size];
			}

			inline int getSize() {
				return _size;
			}
		
		private:

			T*      _buffer;
			int     _index;
			int		_size;

	};

	class AsciiTable {

		public:

			static const int ALIGN_LEFT  = 0;
			static const int ALIGN_RIGHT = 1;
			AsciiTable& reserv(int width) {
				_colSizes[_x] = max(_colSizes[_x], width);
				return *this;
			}

			AsciiTable& up(bool resetColumn = true) {
				_y = _y == 0 ? _bottom : _y - 1;
				if (resetColumn) {
					_x = 0;
				}
				return *this;
			}

			AsciiTable& left() {
				_x = _x == 0 ? _right : _x - 1;
				return *this;
			}

			AsciiTable& down(bool resetColumn = true) {
				_y++;
				if (resetColumn) {
					_x = 0;
				}
				return *this;
			}

			AsciiTable& right() {
				_x++;
				return *this;
			}

			AsciiTable& setCell(int row, int col, std::string str) {
				_x = col;
				_y = row;
				return setCell(str);
			}

			AsciiTable& setCell(std::string str) {
				MARK_FUNC_IN
				while (_y > ((int) _cells.size()) - 1) {
					_cells.push_back(std::vector<std::string>());
				}
				while (_x > ((int) _cells[_y].size()) - 1) {
					_cells[_y].push_back("");
				}
				while (_x > ((int) _colSizes.size()) - 1) {
					_colSizes.push_back(0);
				}
				while (_x > ((int) _colAlign.size()) - 1) {
					_colAlign.push_back(0);
				}

				_cells[_y][_x] = str;
				_colSizes[_x]  = max(_colSizes[_x], str.size());
				MARK_FUNC_OUT

				if (_bottom < _y) {
					_bottom = _y;
				}
				if (_right < _x) {
					_right = _x;
				}
				return *this;
			}

			inline AsciiTable& setAlign(const int col, const int align) {
				_colAlign[col] = align;
				return *this;
			}

			inline const int getRowsCount() {
				return _bottom+1;
			}

			inline const int getColsCount() {
				return _right+1;
			}

			std::string toString() {
				std::stringstream ss;

				auto rc    = getRowsCount();
				auto cc    = getColsCount();
				auto width = getWidth();
				std::string spacer(_cellspacing, ' ');

				auto ir = 0;
				for (auto& row : _cells) {
					auto rw = 0;
					auto ic = 0;
					for (auto& cell : row) {
						ss << (ic > 0 ? spacer : "");

						if (_colAlign[ic] == ALIGN_LEFT) {
							ss << cell << std::string(_colSizes[ic] - cell.size(), ' ');
						}
						else if (_colAlign[ic] == ALIGN_RIGHT) {
							ss << std::string(_colSizes[ic] - cell.size(), ' ') << cell;
						}
						
						rw += _colSizes[ic++];
					}
					ss << std::string(width - rw, ' ');
					if (++ir < rc) {
						ss << std::endl;
					}
				}

				return ss.str();
			}

		private:

			int _x           = 0;
			int _y           = 0;
			int _bottom      = 0;
			int _right       = 0;
			int _cellspacing = 1;

			std::vector< std::vector<std::string> > _cells;
			std::vector< unsigned > _colSizes;
			std::vector< int > _colAlign;

			inline const int getWidth() {
				auto w = _cellspacing * (getColsCount() - 1);
				for (auto n : _colSizes) {
					w += n;
				}
				return w;
			}

	};


}


}