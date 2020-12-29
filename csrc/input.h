#pragma once

#include <memory>
#include "io/io.h"
#include "unicode.h"

class Input {
	private:
		std::shared_ptr<io> io_interface;
		unicode_converter converter;

		void update_key_variables(bool blocking);

	public:
		bool keypressed() const;
		void set_interface(std::shared_ptr<io> io_in) {
			io_interface = io_in;
		}

		short InputDeltaX, InputDeltaY;
		bool InputShiftPressed;
		bool InputSpecialKeyPressed;
		bool InputShiftAccepted;
		short InputKeyPressed;

		void update();
		void wait_for_key();

		short read_key_blocking() {
			wait_for_key();
			return InputKeyPressed;
		}
};