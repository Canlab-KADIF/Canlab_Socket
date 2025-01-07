import socket
import threading
import tkinter as tk
from tkinter import scrolledtext

class ServerApp:
    def __init__(self, master):
        self.master = master
        master.title("서버")

        self.clients = []  # 연결된 클라이언트 목록
        self.message_to_send = None  # 전송할 메시지 저장 변수

        # 버튼을 담을 프레임을 상단에 배치
        button_frame = tk.Frame(master)
        button_frame.pack(padx=10, pady=5)

        # 4개의 메시지 버튼 추가
        self.button1 = tk.Button(button_frame, text="control", width=10, height=3, command=lambda: self.set_message("control"))
        self.button1.pack(side=tk.LEFT, padx=5)

        self.button2 = tk.Button(button_frame, text="perception", width=10, height=3, command=lambda: self.set_message("perception"))
        self.button2.pack(side=tk.LEFT, padx=5)

        self.button3 = tk.Button(button_frame, text="localization", width=10, height=3, command=lambda: self.set_message("localization"))
        self.button3.pack(side=tk.LEFT, padx=5)

        self.button4 = tk.Button(button_frame, text="planning", width=10, height=3, command=lambda: self.set_message("planning"))
        self.button4.pack(side=tk.LEFT, padx=5)

        # 스크롤 가능한 텍스트 영역
        self.text_area = scrolledtext.ScrolledText(master, wrap=tk.WORD, width=50, height=10)
        self.text_area.pack(padx=10, pady=10, fill=tk.BOTH, expand=True)

        # 사용자 입력 필드를 전송 버튼 위에 배치
        self.entry_field = tk.Entry(master, width=50)
        self.entry_field.pack(padx=10, pady=5)

        # 전송 버튼 추가
        self.send_button = tk.Button(master, text="confirm", command=self.send_user_message)
        self.send_button.pack(pady=5)

        self.start_server()

    def start_server(self):
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.bind(('127.0.0.1', 12345))
        self.server_socket.listen()

        self.text_area.insert(tk.END, "서버가 시작되었습니다...\n")
        self.accept_thread = threading.Thread(target=self.accept_connections)
        self.accept_thread.start()

    def accept_connections(self):
        while True:
            conn, addr = self.server_socket.accept()
            self.clients.append(conn)  # 클라이언트를 리스트에 추가
            self.text_area.insert(tk.END, f"{addr}가 연결되었습니다.\n")
            threading.Thread(target=self.handle_client, args=(conn,)).start()

    def handle_client(self, conn):
        # 클라이언트에게 인사 메시지 송신
        greeting_message = "Hello, Client!"
        conn.sendall(greeting_message.encode())

        while True:
            try:
                data = conn.recv(1024)  # 클라이언트로부터 데이터 수신
                if not data:
                    break  # 데이터가 없으면 클라이언트가 연결을 종료한 것
                message = data.decode('utf-8')  # 데이터를 문자열로 디코딩
                # 클라이언트 메시지를 텍스트 영역에 출력
                self.text_area.insert(tk.END, f"클라이언트로부터 받은 메시지: {message}\n")
            except Exception as e:
                self.text_area.insert(tk.END, f"클라이언트 통신 오류: {e}\n")
                break

        conn.close()
        self.clients.remove(conn)  # 클라이언트가 연결을 끊으면 목록에서 제거

    def set_message(self, message):
        # 메시지 버튼 클릭 시 메시지를 저장
        self.message_to_send = message
        self.text_area.insert(tk.END, f"선택된 메시지: {message}\n")

    def send_user_message(self):
        # 입력 필드 또는 버튼에서 선택된 메시지를 클라이언트에게 전송
        message = self.entry_field.get() if self.entry_field.get() else self.message_to_send
        if message:
            self.text_area.insert(tk.END, f"서버에서 보낸 메시지: {message}\n")
            for client in self.clients:
                try:
                    client.sendall(message.encode())
                except Exception as e:
                    self.text_area.insert(tk.END, f"메시지 전송 실패: {e}\n")
                    self.clients.remove(client)  # 실패한 클라이언트 제거
            self.entry_field.delete(0, tk.END)  # 입력 필드 초기화
            self.message_to_send = None  # 메시지 전송 후 변수 초기화
        else:
            self.text_area.insert(tk.END, "전송할 메시지가 없습니다.\n")

if __name__ == "__main__":
    root = tk.Tk()
    root.title("adnomal Selector")
    root.geometry("500x400")
    app = ServerApp(root)
    root.mainloop()

