window: window.cc my_file_dir.cc
	/usr/local/hdf5/bin/h5c++ -o window window.cc my_file_dir.cc /usr/local/lib/libfltk.a -lpthread -framework Cocoa -I/Users/yoshi/include  -std=c++11

mib-window: mib_window.cc 
	g++ -o mib_window mib_window.cc /usr/local/lib/libfltk.a -lpthread -framework Cocoa -I/Users/yoshi/include  -std=c++11 -framework Accelerate /Users/yoshi/lib/Savitzky_Golay_2d.o -lfftw3 peak_picker.cc

mib_window_x86: mib_window.cc
	clang -o mib_window_x86_64 mib_window.cc  -mmacosx-version-min=10.15 -arch x86_64 /usr/local/lib/libfltk.a -lpthread -framework Cocoa -I/Users/yoshi/include  -std=c++11 -framework Accelerate /Users/yoshi/lib/Savitzky_Golay_2d.o -lfftw3 peak_picker.cc

test: test.cc
	g++ -o test test.cc my_file_dir.cc /usr/local/lib/libfltk.a -lpthread -framework Cocoa -I/Users/yoshi/include 
test2: test2.cc
	/usr/local/hdf5/bin/h5c++ -o test2 test2.cc -I/opt/homebrew/include/ -I/Users/yoshi/softwares/h5xx -std=c++11  

my_ls: my_h5ls.c
	/usr/local/hdf5/bin/h5cc -o myls my_h5ls.c  -I/Users/yoshi/softwares/hdf5-1.14.2/tools/lib -I/Users/yoshi/softwares/hdf5-1.14.2/src/ /Users/yoshi/softwares/hdf5-1.14.2/tools/lib/h5tools.o /Users/yoshi/softwares/hdf5-1.14.2/tools/lib/h5tools_dump.o /Users/yoshi/softwares/hdf5-1.14.2/tools/lib/h5tools_ref.o /Users/yoshi/softwares/hdf5-1.14.2/tools/lib/h5tools_utils.o /Users/yoshi/softwares/hdf5-1.14.2/tools/lib/h5trav.o /Users/yoshi/softwares/hdf5-1.14.2/tools/lib/h5tools_str.o /Users/yoshi/softwares/hdf5-1.14.2/tools/lib/h5tools_type.o

tool: tool.cc
	/usr/local/hdf5/bin/h5c++ -o tool tool.cc  -I/Users/yoshi/softwares/hdf5-1.14.2/tools/lib -I/Users/yoshi/softwares/hdf5-1.14.2/src/ /Users/yoshi/softwares/hdf5-1.14.2/tools/lib/h5tools.o /Users/yoshi/softwares/hdf5-1.14.2/tools/lib/h5tools_dump.o /Users/yoshi/softwares/hdf5-1.14.2/tools/lib/h5tools_ref.o /Users/yoshi/softwares/hdf5-1.14.2/tools/lib/h5tools_utils.o /Users/yoshi/softwares/hdf5-1.14.2/tools/lib/h5trav.o /Users/yoshi/softwares/hdf5-1.14.2/tools/lib/h5tools_str.o /Users/yoshi/softwares/hdf5-1.14.2/tools/lib/h5tools_type.o -I/Users/yoshi/softwares/hdf5-1.14.2/src/H5FDsubfiling
