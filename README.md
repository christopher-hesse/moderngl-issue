```
docker build . --tag egl_test
docker run -it --rm egl_test bash -c "gcc egl_test.c -lEGL -lOpenGL && ./a.out && tail out.ppm"
docker run -it --rm egl_test python egl_test.py
```