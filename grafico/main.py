import re
import pandas as pd
import matplotlib.pyplot as plt
import tkinter as tk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.backends.backend_tkagg import NavigationToolbar2Tk

class ToolBar(NavigationToolbar2Tk):
    def __init__(self, canvas, window, pack_toolbar=True):
        super().__init__(canvas, window, pack_toolbar=pack_toolbar)
        self.update()
        self.pack()
        self.message = tk.Label()
        self.message.config(text="x=0.0 \n y=None")
        self.message.pack()
    def set_message(self, msg):
        if msg != "":
            self.message.config(text=msg.replace(" ", "\n"))

class Chart:
    def __init__(self):
        with open("processos.txt", "r") as f:
            self.cpus = [str(x) for x in range(int(f.readline().split("=")[1]))]
        self.i = 0
        self.current_cpu = self.cpus[self.i % len(self.cpus)]
        self.title = f'Trocas de processo (sched_switch) no core {int(self.current_cpu)+1} ao longo do tempo'
        self.fig, self.ax = plt.subplots(figsize=(10, 5))

    def next(self):
        self.i += 1
        self.current_cpu = self.cpus[self.i % len(self.cpus)]
        self.title = f'Trocas de processo (sched_switch) no core {int(self.current_cpu)+1} ao longo do tempo'
        self.plot()

    def previous(self):
        self.i -= 1
        self.current_cpu = self.cpus[self.i % len(self.cpus)]
        self.title = f'Trocas de processo (sched_switch) no core {int(self.current_cpu)+1} ao longo do tempo'
        self.plot()

    def generate_data(self, selected_core):
        switch = []
        self.wakeup = []
        with open('processos.txt', 'r') as file:
            for line in file:
                # Regex para capturar o core, timestamp e o evento de sched_switch
                match = re.search(r'\[([\d]+)\] +(\d+\.\d+): sched_switch: .*? ==> (.*?) ', line)
                if match:
                    core = match.group(1)
                    timestamp = float(match.group(2))
                    next_process = match.group(3)
                    
                    # Filtrar pelos dados do core selecionado
                    if core == selected_core:
                        switch.append((timestamp, next_process))

                match = re.search(r'\[([\d]+)\] +(\d+\.\d+): sched_wakeup: *(.*?) ', line)
                if match:
                    core = match.group(1)
                    timestamp = float(match.group(2))
                    process_name = match.group(3)

                    if core == selected_core:
                        self.wakeup.append((timestamp, process_name))

        
        self.values = []
        for i in range(len(switch)):
            if 'swapper' in switch[i][1]:
                continue
            if i < len(switch) - 1:
                self.values.append((switch[i][0], switch[i][1]))
                self.values.append((switch[i+1][0], switch[i][1]))

    def plot(self):
        self.ax.clear()
        self.generate_data("00" + self.current_cpu)
        df = pd.DataFrame(self.wakeup, columns=['timestamp', 'process_name'])
        df2 = pd.DataFrame(self.values, columns=['timestamp', 'next_process'])
        
        for i in range(len(df) - 1):
            self.ax.plot(df['timestamp'][i:i+2], df['process_name'][i:i+2], color='green', marker='o', linestyle='')
        
        for i in range(len(df2) - 1):
            if i % 2 == 0:
                self.ax.plot(df2['timestamp'][i:i+2], df2['next_process'][i:i+2], color='skyblue', marker='o')
            else:
                self.ax.plot(df2['timestamp'][i:i+2], df2['next_process'][i:i+2], color='skyblue', marker='o', linestyle='')

        self.ax.set_xlabel('Tempo (s)')
        self.ax.set_ylabel('Próximo processo')
        self.ax.tick_params(axis='x', rotation=45)
        self.ax.grid(True)
        self.ax.plot()
        label.config(text=self.title)
        canvas.draw()

chart = Chart()
root = tk.Tk()
frame = tk.Frame(root)
label = tk.Label(text=chart.title)
label.pack()

canvas = FigureCanvasTkAgg(chart.fig, master=root)
canvas.get_tk_widget().pack(ipadx=100, ipady=50)


toolbar = ToolBar(canvas, frame, pack_toolbar=False)


frame.pack()
chart.plot()
tk.Button(frame, text="Anterior", command=chart.previous).pack(side="left")
tk.Button(frame, text="Próximo", command=chart.next).pack()
root.mainloop()
