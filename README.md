# football-sim
A basic football simulator, using a softmax regression model to call plays.

## Disclaimer
This is an old project that I'm attempting to revive. As a result of its age, there is a lot of work for me to do to clean this up to use modern C++ best practices. Bear with me.

## Build
This uses ```cmake``` to build. From the root project directory, you can do
```
cmake -S ./ -B build/
cd build/
make
```

This will build the project and train the playcall model. At this point, you can run the driver program with ```driver```.
