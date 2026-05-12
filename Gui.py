import tkinter as tk
from tkinter import scrolledtext
import subprocess
import threading
import queue
from PIL import Image, ImageTk

BG        = "#1e1e1e"
BG2       = "#252526"
BG3       = "#2d2d2d"
BORDER    = "#404040"
FG        = "#f8e9cd"
FG_DIM    = "#858585"
GREEN     = "#a46ca8"
BLUE      = "#a46ca8"
Novacolor = "#42569e"
ORANGE    = "#ce9178"
RED       = "#f48771"
BTN_GREEN = "#0e7c3a"

class NovaIDE:
    def __init__(self, root):
        self.root = root
        self.root.title("Nova Compiler")
        self.root.geometry("1400x800")
        self.root.configure(bg=BG3)
        self.root.minsize(800, 500)
        self.process = None
        self._build_ui()

    def _build_ui(self):
        mono = ("Consolas", 12)

        top = tk.Frame(self.root, bg=BG3, pady=6)
        top.pack(fill="x")

        tk.Label(top, text="🚀 NOVA", bg=BG3, fg=Novacolor,
                font=("Consolas", 13, "bold")).pack(side="left", padx=14)
        tk.Label(top, text="test.nova", bg=BG, fg=FG_DIM,
                font=("Consolas", 11), padx=12, pady=4).pack(side="left")

        self.status_lbl = tk.Label(top, text="● Ready", bg=BG3,
                                fg=Novacolor, font=("Consolas", 11))
        self.status_lbl.pack(side="left", padx=20)

        tk.Button(top, text="Clear", bg=BG2, fg=FG_DIM, font=mono,
                relief="flat", padx=10, command=self.clear_output,
                cursor="hand2").pack(side="right", padx=6)

        self.run_btn = tk.Button(
            top, text="▶  Run", bg=BTN_GREEN, fg="white",
            font=("Consolas", 12, "bold"), relief="flat",
            padx=16, pady=4, command=self.run_code, cursor="hand2")
        self.run_btn.pack(side="right", padx=6)

        tk.Frame(self.root, bg=BORDER, height=1).pack(fill="x")

        # ── main split ──
        panes = tk.PanedWindow(self.root, orient="horizontal",
                            bg=BORDER, sashwidth=4, sashrelief="flat")
        panes.pack(fill="both", expand=True)

        # editor
        left = tk.Frame(panes, bg=BG)
        panes.add(left, minsize=300)
        tk.Label(left, text="EDITOR  —  test.nova", bg=BG2, fg=FG_DIM,
                font=("Consolas", 10), anchor="w",
                padx=10, pady=4).pack(fill="x")
        self.editor = scrolledtext.ScrolledText(
            left, bg=BG, fg=FG, insertbackground="white",
            font=mono, relief="flat", bd=0,
            selectbackground="#264f78", wrap="none")
        self.editor.pack(fill="both", expand=True, padx=2, pady=2)
        self.editor.insert("1.0", 'module launch() {\n    transmit "Hello, Nova!";\n}')

        # terminal
        right = tk.Frame(panes, bg=BG)
        panes.add(right, minsize=300)
        tk.Label(right, text="TERMINAL  OUTPUT", bg=BG2, fg=FG_DIM,
                font=("Consolas", 10), anchor="w",
                padx=10, pady=4).pack(fill="x")

        self.terminal = scrolledtext.ScrolledText(
            right, bg="#1a1a1a", fg=FG, insertbackground="white",
            font=mono, relief="flat", bd=0, state="disabled",
            selectbackground="#264f78")
        self.terminal.pack(fill="both", expand=True, padx=2, pady=2)

        self.terminal.tag_config("info",    foreground=BLUE)
        self.terminal.tag_config("success", foreground=GREEN)
        self.terminal.tag_config("err",     foreground=RED)
        self.terminal.tag_config("inp",     foreground=ORANGE)
        self.terminal.tag_config("normal",  foreground=FG)

        # input bar (always visible at bottom of terminal)
        input_frame = tk.Frame(right, bg=BG2)
        input_frame.pack(fill="x", side="bottom")
        tk.Frame(right, bg=BORDER, height=1).pack(fill="x", side="bottom")

        tk.Label(input_frame, text="›", bg=BG2, fg=GREEN,
                font=("Consolas", 14)).pack(side="left", padx=8)

        self.input_var = tk.StringVar()
        self.input_entry = tk.Entry(
            input_frame, textvariable=self.input_var,
            bg=BG2, fg=ORANGE, insertbackground=ORANGE,
            font=mono, relief="flat", bd=0,
            disabledbackground=BG2)
        self.input_entry.pack(side="left", fill="x", expand=True, pady=7)
        self.input_entry.bind("<Return>", self.send_input)
        self.input_entry.configure(state="disabled")

        tk.Button(input_frame, text="⏎", bg=BG2, fg=BLUE,
                font=("Consolas", 14), relief="flat",
                command=self.send_input, cursor="hand2").pack(side="right", padx=8)

        # status bar
        sb = tk.Frame(self.root, bg="#007acc", height=22)
        sb.pack(fill="x", side="bottom")
        tk.Label(sb, text="Nova Language", bg="#007acc", fg="white",
                font=("Consolas", 10), padx=10).pack(side="right")

        self._term_write("Nova Compiler v1.0 — ready\n", "info")
        self._term_write("─" * 40 + "\n", "info")

    # ── helpers ───────────────────────────────────────────
    def _term_write(self, text, tag="normal"):
        self.terminal.configure(state="normal")
        self.terminal.insert("end", text, tag)
        self.terminal.see("end")
        self.terminal.configure(state="disabled")

    def set_status(self, text, color=FG_DIM):
        self.status_lbl.configure(text=text, fg=color)

    def clear_output(self):
        self.terminal.configure(state="normal")
        self.terminal.delete("1.0", "end")
        self.terminal.configure(state="disabled")
        self._term_write("Nova Compiler v1.0 — ready\n", "info")
        self._term_write("─" * 40 + "\n", "info")

    # ── input ─────────────────────────────────────────────
    def send_input(self, event=None):
        if not self.process or self.process.poll() is not None:
            return
        val = self.input_var.get()
        self.input_var.set("")
        self._term_write("› " + val + "\n", "inp")
        try:
            self.process.stdin.write(val + "\n")
            self.process.stdin.flush()
        except Exception as e:
            self._term_write(f"Input error: {e}\n", "err")


    def run_code(self):
    
        if self.process and self.process.poll() is None:
            self.process.kill()

        code = self.editor.get("1.0", "end-1c")
        with open("test.nova", "w", encoding="utf-8") as f:
            f.write(code)

        self.clear_output()
        self._term_write("▶ Compiling & Running...\n\n", "info")
        self.run_btn.configure(state="disabled", text="⟳  Running")
        self.set_status("⟳ Running...", BLUE)
        self.input_entry.configure(state="normal")
        self.input_entry.focus_set()

        threading.Thread(target=self._run_process, daemon=True).start()

    def _run_process(self):
        try:
            self.process = subprocess.Popen(
                ["compiler.exe"],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,  
                text=True,
                bufsize=0                   
            )

            buf = ""
            while True:
                ch = self.process.stdout.read(1)
                if not ch:
                    break
                buf += ch
                if ch in ("\n", "\r"):
                    line = buf.rstrip()
                    buf = ""
                    if line:
                        self.root.after(0, self._term_write, line + "\n", "normal")

            self.process.wait()
            rc = self.process.returncode
            self.root.after(0, self._on_done, rc)

        except FileNotFoundError:
            self.root.after(0, self._term_write,
                            "✗ compiler.exe not found!\n", "err")
            self.root.after(0, self._on_done, -1)
        except Exception as e:
            self.root.after(0, self._term_write, f"✗ {e}\n", "err")
            self.root.after(0, self._on_done, -1)

    def _on_done(self, rc):
        self.input_entry.configure(state="disabled")
        self._term_write("\n" + "─" * 40 + "\n", "info")
        if rc == 0:
            self._term_write("✓ Program finished successfully\n", "success")
            self.set_status("✓ Done", GREEN)
        else:
            self._term_write(f"✗ Exited with code {rc}\n", "err")
            self.set_status("✗ Error", RED)
        self.run_btn.configure(state="normal", text="▶  Run")
        self.process = None


if __name__ == "__main__":
    root = tk.Tk()
    app = NovaIDE(root)
    root.mainloop()