#pragma once

#include "debug/Debug.h"

namespace fxc {

namespace utils {
	
	// ��������� ����������� ������-��������� � ����� mql4
	template <typename T>
	class CircularBuffer {

		public:

			CircularBuffer() {
			}
			
			~CircularBuffer() {
				delete []_buffer;
			}

			void alloc(int size) {
				MARK_FUNC_IN
				if (_buffer == nullptr) {
					_buffer = new T[size];
					memset(_buffer, 0, sizeof(T) * size);
				}
				else {
					auto b = new T[size];
					
					if (size > _size - _index) {
						memcpy(b, _buffer + _index, _size - _index);
						memcpy(b + _size - _index, _buffer, size - _size + _index);
					}
					else {
						memcpy(b, _buffer + _index, size);
					}

					delete []_buffer;
					_buffer = b;
				}
				_size  = size;
				_index = 0;
				MARK_FUNC_OUT
			}

			void add(T value) {
				_index = _index ? _index-1 : _size-1;
				_buffer[_index] = value;
			}
			
			void update(T value) {
				_buffer[_index] = value;
			}

			void skip(int i) {
				i = i % _size;
				_index = i > _index ? 
					_index - i + _size : 
					_index - i;
			}
			
			T& operator[](const int i) {
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

}

}