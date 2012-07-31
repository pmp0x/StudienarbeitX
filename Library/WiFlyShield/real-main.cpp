#include <iostream>
#include <iterator>
#include <vector>

void setup(){
	
	while(SerialUSB.available());
	
	vector<int> myvector;
	  for (int i=1; i<10; ++i) myvector.push_back(i*10);

	  ostream_iterator<int> out_it (SerialUSB.write(),", ");
	  copy ( myvector.begin(), myvector.end(), out_it );
}

void loop()
{
    



}

// Standard libmaple init() and main.
//
// The init() part makes sure your board gets set up correctly. It's
// best to leave that alone unless you know what you're doing. main()
// is the usual "call setup(), then loop() forever", but of course can
// be whatever you want.

__attribute__((constructor)) void premain() {
    init();
}

int main(void) {
    setup();

    while (true) {
        loop();
    }

    return 0;
}