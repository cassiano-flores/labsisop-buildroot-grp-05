import os
import sys
import time
import socket
from http.server import BaseHTTPRequestHandler,HTTPServer

HOST_NAME = '192.168.1.10' # !!!REMEMBER TO CHANGE THIS!!!
PORT_NUMBER = 8000

def get_time():
    return os.popen("date").read()

def get_uptime():
    with open('/proc/uptime', 'r') as f:
        uptime_seconds = float(f.readline().split()[0])
        return uptime_seconds

def get_cpu_info():
    with open('/proc/cpuinfo', 'r') as f:
        cpuinfo = f.readlines()

    model_name = [line.split(':')[-1].strip() for line in cpuinfo if 'model name' in line][0]
    mhz = [line.split(':')[-1].strip() for line in cpuinfo if 'cpu MHz' in line][0]
    return model_name, mhz

def get_cpu_percent():
    with open('/proc/stat', 'r') as f:
        fields = [float(field) for field in f.readline().strip().split()[1:]]
        idle_time = fields[3]
        total_time = sum(fields)
        return (1.0 - idle_time / total_time) * 100.0

def get_mem_info():
    with open('/proc/meminfo', 'r') as f:
        meminfo = f.readlines()

    total_memory = int([line.split(':')[-1].strip() for line in meminfo if 'MemTotal' in line][0].split()[0]) // 1024

    free = int([line.split(':')[-1].strip() for line in meminfo if 'MemFree' in line][0].split()[0])

    buffers = int([line.split(':')[-1].strip() for line in meminfo if 'Buffers' in line][0].split()[0])

    cached = int([line.split(':')[-1].strip() for line in meminfo if 'Cached' in line][0].split()[0])

    used_memory =   ((total_memory - free - buffers - cached) // 1024) * -1

    return total_memory, used_memory

def get_processes():
    processes = []
    for pid in os.listdir('/proc'):
        try:
            pid = int(pid)
            with open(os.path.join('/proc', str(pid), 'stat'), 'rb') as f:
                cmdline = f.read().decode().split(" ")
                if cmdline:
                    processes.append({'pid': cmdline[0], 'name': cmdline[1].strip("()")})
        except (ValueError, FileNotFoundError):
            pass
    return processes

def get_system_info():
    hostname = socket.gethostname()
    os_name = os.uname().sysname
    os_version = os.uname().release

    return hostname, os_name, os_version

def format_time(seconds):
    m, s = divmod(seconds, 60)
    h, m = divmod(m, 60)
    return f"{h:.2f}:{m:.2f}:{s:.2f} ."

def format_processes(processes):
    return '\n'.join([f"{p['pid']}: {p['name']}" for p in processes])

def generate_html():
    uptime = get_uptime()
    cpu_model, cpu_mhz = get_cpu_info()
    cpu_percent = get_cpu_percent()
    total_memory, used_memory = get_mem_info()
    processes = get_processes()
    hostname, os_name, os_version = get_system_info()

    html = f"""
    <html>
        <head>
            <title>Informacoes do Sistema</title>
            <meta charset="UTF-8">
        </head>
        <body>
            <h1>Informacoes do Sistema</h1>
            <h2>Integrantes: (GRUPO-05)</h2>
            <h3>Cassiano Luis Flores Michel - 20204012-7</h3>
            <h3>Mateus de Carvalho de Freitas - 20204015-7</h3>
            <h3>Pedro Menuzzi Mascaró - 20103702-5</h3>
            <h3>Gustavo Geyer Arrussul Winkler dos Santos - 19102825-7</h3>
            <ul>
                <li>Data e hora do sistema: {get_time()}</li>
                <li>Tempo de funcionamento sem reinicializacao do sistema: {format_time(uptime)}</li>
                <li>Modelo do processador: {cpu_model}</li>
                <li>Velocidade do processador: {cpu_mhz} MHz</li>
                <li>Capacidade ocupada do processador: {cpu_percent:.2f}%</li>
                <li>Quantidade de memoria RAM total: {total_memory} MB</li>
                <li>Quantidade de memoria RAM usada: {used_memory} MB</li>
                <li>Versao do sistema: {os_name} {os_version}</li>
                <li>Lista de processos em execucao: <pre>{format_processes(processes)}</pre></li>
            </ul>
        </body>
    </html>
    """
    return html

class MyHandler(BaseHTTPRequestHandler):
    def do_HEAD(s):
        s.send_response(200)
        s.send_header("Content-type", "text/html")
        s.end_headers()
    def do_GET(s):
        """Respond to a GET request."""
        s.send_response(200)
        s.send_header("Content-type", "text/html")
        s.end_headers()
        s.wfile.write(generate_html().encode())

if __name__ == '__main__':
    httpd = HTTPServer((HOST_NAME, PORT_NUMBER), MyHandler)
    print("Server Starts - %s:%s" % (HOST_NAME, PORT_NUMBER))
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass
    httpd.server_close()
    print("Server Stops - %s:%s" % (HOST_NAME, PORT_NUMBER))
