FROM ubuntu:bionic-20190122
RUN apt-get update
RUN apt-get install --yes curl build-essential

# python
RUN curl -O https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh
RUN sh Miniconda3-latest-Linux-x86_64.sh -b
ENV PATH=/root/miniconda3/bin:$PATH
ENV PYTHONDONTWRITEBYTECODE=1
ENV PYTHONUNBUFFERED=1
RUN conda update conda
ADD env.yaml .
RUN conda env update --name env --file env.yaml
ENV PATH=/root/miniconda3/envs/env/bin:$PATH

# egl
RUN apt-get install --yes libegl1-mesa libegl1-mesa-dev libgl1-mesa-glx

RUN pip install moderngl==5.6.0

ADD egl_test.c .
ADD egl_test.py .