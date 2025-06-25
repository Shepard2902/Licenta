import socket
import struct
import threading
from collections import deque
import matplotlib

matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
import matplotlib.animation as animation

# Multicast connection for data readings
MCAST_GRP = '239.1.2.3'
MCAST_PORT = 1234
BUFFER_SIZE = 1024

# Plot settings
MAX_POINTS = 500
master_emg_data = deque([0.0] * MAX_POINTS, maxlen=MAX_POINTS)
slave_emg_data = deque([0.0] * MAX_POINTS, maxlen=MAX_POINTS)
master_raw_data = deque([0.0] * MAX_POINTS, maxlen=MAX_POINTS)  # Added
slave_raw_data = deque([0.0] * MAX_POINTS, maxlen=MAX_POINTS)  # Added

# Data readings from UDP multicasting connection
def listen_udp():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(('', MCAST_PORT))
    mreq = struct.pack("4sl", socket.inet_aton(MCAST_GRP), socket.INADDR_ANY)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

    while True:
        data, addr = sock.recvfrom(BUFFER_SIZE)
        message = data.decode('utf-8')


        if message.startswith("RAW:"):
            try:
                value = float(message.split(":")[1])
                if addr[0] == '192.168.4.1':
                    master_raw_data.append(value)
                else:
                    slave_raw_data.append(value)
            except ValueError:
                pass

        elif message.startswith("EMG:"):
            try:
                value = float(message.split(":")[1])
                if addr[0] == '192.168.4.1':
                    master_emg_data.append(value)
                else:
                    slave_emg_data.append(value)
            except ValueError:
                pass


# Filtered and raw data plot
def update_plot(frame, lines, axes):
    lines[0].set_ydata(list(master_emg_data))
    lines[0].set_xdata(range(len(master_emg_data)))
    lines[1].set_ydata(list(slave_emg_data))
    lines[1].set_xdata(range(len(slave_emg_data)))
    axes[0].relim()
    axes[0].autoscale_view()
    axes[1].relim()
    axes[1].autoscale_view()
    return lines


def main():
    thread = threading.Thread(target=listen_udp, daemon=True)
    thread.start()

    fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True)
    fig.suptitle("EMG Data (Filtered)")

    line1, = ax1.plot(range(MAX_POINTS), list(master_emg_data), 'b')
    line2, = ax2.plot(range(MAX_POINTS), list(slave_emg_data), 'r')

    ax1.set_title("Master")
    ax1.set_ylabel("Filtered Value")
    ax1.set_ylim(0, 2000)
    ax2.set_title("Slave")
    ax2.set_xlabel("Sample")
    ax2.set_ylabel("Filtered Value")
    ax2.set_ylim(0, 2000)

    ani = animation.FuncAnimation(
        fig, update_plot, fargs=([line1, line2], [ax1, ax2]),
        interval=20, cache_frame_data=False
    )
    fig.ani = ani


    raw_fig, (ax3, ax4) = plt.subplots(2, 1, sharex=True)
    raw_fig.suptitle("EMG Raw Data")
    line3, = ax3.plot(range(MAX_POINTS), list(master_raw_data), 'g')
    line4, = ax4.plot(range(MAX_POINTS), list(slave_raw_data), 'm')

    ax3.set_title("Master")
    ax3.set_ylabel("Raw Value")
    ax3.set_ylim(0, 5000)
    ax4.set_title("Slave")
    ax4.set_xlabel("Sample")
    ax4.set_ylabel("Raw Value")
    ax4.set_ylim(0, 5000)

    def update_raw_plot(frame):
        line3.set_ydata(list(master_raw_data))
        line4.set_ydata(list(slave_raw_data))
        return line3, line4

    raw_ani = animation.FuncAnimation(
        raw_fig, update_raw_plot, interval=20, blit=True
    )
    raw_fig.ani = raw_ani

    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    main()