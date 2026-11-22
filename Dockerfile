FROM python:3.12-slim

RUN apt-get update && apt-get install -y build-essential && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY . .

RUN gcc -c -Iinclude src/c_modules/*.c \
 && g++ -Iinclude -o routing_cli src/cpp_modules/routing_cli.cpp src/cpp_modules/RoutingSystem.cpp *.o

CMD ["python", "gui/server.py"]