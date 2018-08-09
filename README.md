# Moviment_Detector
Detect Moviment And Save Images

g++ -std=c++11 main.cpp -o main.o `pkg-config --cflags --libs opencv` -lpthread

./main.o MovEvents 10.110.1.56:554/cam/realmonitor?channel=1&subtype=1
