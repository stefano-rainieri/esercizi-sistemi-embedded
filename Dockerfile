FROM ubuntu:latest
RUN mkdir -p /var/www/app
ADD . /var/www/app
WORKDIR /var/www/app
RUN apt-get update -y && apt-get install -y build-essential vim 
