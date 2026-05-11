FROM ubuntu:latest

WORKDIR /app

RUN apt update && \
    apt install -y gcc fuse libfuse-dev

COPY . .

RUN chmod +x server

EXPOSE 9000

CMD ["./server"]
