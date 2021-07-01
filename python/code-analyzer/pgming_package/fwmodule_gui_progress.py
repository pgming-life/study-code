"""fwmodule_gui_progress"""

import threading
import datetime as dt
import tkinter as tk
import tkinter.ttk

flag_release = False

try:
    import pgming_package.release as r
except Exception:
    # エラースルー
    pass
else:
    flag_release = r.flag

# 上記のインポートが読み込めない場合はflag_releaseは基本的にFalse(存在しない変数であるため)
if flag_release:    # リリース用インポート
    from pgming_package.fwmodule_general import *
else:               # デバッグ用インポート
    # ルートパス可変制御
    import sys, os      # fwdef.pyと同じモジュールインポートだが、Python内部でインクルードガード出来ているので問題ない。
    if not [i for i in sys.path if i == os.path.dirname(__file__)]:
        sys.path.append(os.path.dirname(__file__))
    from fwmodule_general import *

#####################################################################
"""
    GUIプログレスレシーバー
    備考: サブ計算(非確定的処理)

"""
class ProgressReceiver:
    def __init__(self, place_x, place_y):
        self.flag_loop = False          # ループフラグ
        self.flag_progress = False      # プログレスフラグ
        self.place_x = place_x          # プログレス位置x
        self.place_y = place_y          # プログレス位置y
        self.y_margin = 17              # 間隔y
        self.sec = 0.2                  # プログレス間隔

        # アスキーアート
        self.label_python = []
        self.label_python.append(tk.Label(text="                        (    I      I     I    )"))
        self.label_python.append(tk.Label(text="                        (    I     III    I    )"))
        self.label_python.append(tk.Label(text="                     (    I        III       I    )"))
        self.label_python.append(tk.Label(text="                  ( (  I        I      I        I  ) )"))
        self.label_python.append(tk.Label(text="              ( (  I        I              I        I  ) )"))
        self.label_python.append(tk.Label(text="             ( ( I         I                I         I ) )"))
        self.label_python.append(tk.Label(text="            ( (   I            I        I            I   ) )"))
        self.label_python.append(tk.Label(text="             (   ( I             I    I             I )   )"))
        self.label_python.append(tk.Label(text="              ( ( (  I I           I           I I  ) ) )"))
        self.label_python.append(tk.Label(text="                 ( (( I    I               I    I )) )"))
        self.label_python.append(tk.Label(text="                    ( [(Θ))  I      I  ((Θ)] )"))
        self.label_python.append(tk.Label(text="                      ( (I                    I) )"))
        self.label_python.append(tk.Label(text="                        ( (I                I) )"))
        self.label_python.append(tk.Label(text="                          (    o      o    )"))
        self.label_python.append(tk.Label(text="                            (  __人__  )"))
        cnt_y_margin = counter(self.place_y, self.y_margin)
        for i in self.label_python:
            i.place(x=self.place_x, y=cnt_y_margin.result())
            cnt_y_margin.count()

        # 変化ラベル
        self.lines_move = []
        self.lines_move.append("                                   ((")
        self.lines_move.append("                                   ))")
        self.lines_move.append("                                   ((")
        self.lines_move.append("                                   ))")
        self.lines_move.append("                                   ((")
        self.lines_move.append("                                   )(")

        # 終了ラベル
        self.lines_end = []
        self.lines_end.append("                                   ^ ")
        self.lines_end.append("                            Complete!!")
        self.lines_end.append("                                     ")
        self.lines_end.append("                                     ")
        self.lines_end.append("                                     ")
        self.lines_end.append("                                     ")

        # ラベル行数が一致しない場合にアサート(デバッグ用)
        #assert len(self.lines_move) == len(self.lines_end), "変化ラベルと終了ラベルの行数が一致しません。"
        
        # ヒープ領域を確保し変化ラベル位置を設定
        self.label_python_move = [tk.Label(text="") for i in range(len(self.lines_move))]
        cnt_y_margin = counter(self.place_y+self.y_margin*len(self.label_python), self.y_margin)
        for i in self.label_python_move:
            i.place(x=self.place_x, y=cnt_y_margin.result())
            cnt_y_margin.count()

    # プログレススレッドスタート
    def progress_start(self):
        while self.flag_loop:
            try:
                time.sleep(self.sec)
                if self.flag_progress:
                    for i, j in enumerate(self.label_python_move):
                        if self.flag_loop:      # スレッド重複処理回避
                            j['text'] = self.lines_move[i]
                            time.sleep(self.sec)
                        else:
                            break
                else:
                    # 全消去
                    for i in self.label_python_move:
                        if self.flag_loop:      # 念のため
                            i['text'] = ""
                        else:
                            break
                self.flag_progress = not self.flag_progress
            except Exception:
                # Tkinter強制終了エラー回避(保険)
                pass
                
        try:
            # プログレス終了処理
            for i, j in enumerate(self.label_python_move):
                j['text'] = self.lines_end[i]
        except Exception:
            # Tkinter強制終了エラー回避(保険)
            pass

    # スレッドスタート
    def start(self):
        self.thread_progress = threading.Thread(target = self.progress_start)
        self.thread_progress.setDaemon(True)
        self.thread_progress.start()

#####################################################################
"""
    GUIプログレスバー
    備考: メイン計算(確定的処理)

"""
class Progressbar:
    def __init__(self, self_root, bar_x, bar_y, bar_len, i=100):
        self.set = tk.ttk.Progressbar(self_root, length=bar_len)
        self.set.configure(value=0, mode="determinate", maximum=i)
        self.set.place(x=bar_x, y=bar_y)

    # 更新
    def update(self, i):
        self.set.configure(value=i+1)      # +1した値を代入することで進捗が末尾まで正常に動作する
        self.set.update()

    # リセット
    def reset(self):
        self.set.stop()

#####################################################################
"""
    進捗ラベル
    備考: 処理内容を表示

"""
class ProgressLabel:
    def __init__(self, place_x, place_y, text_ready=""):
        if text_ready == "":
            text_ready = "処理を開始できます..."
        self.label_progress = tk.Label(text=text_ready)
        self.label_progress.place(x=place_x, y=place_y)
        self.start = time.time()    # 経過時間を測定するための開始時間(プログラム開始)
        self.i = 0

    # 更新(for文中やプログレスバーに挿入)
    def update(self, text_update=""):
        if self.i == 0:         # 経過時間を測定するための開始時間
            self.start = time.time()
        self.i += 1
        if text_update == "":   # 進捗内容を表示しない場合は繰り返し数を表示
            text_update = str(self.i)

        # 「character U + d4b53」が、Tclで許可されている範囲（U + 0000-U + FFFF）を超えている場合表示しない。
        try:
            self.label_progress['text'] = text_update
        except Exception as err:
            #print("エラー内容:")
            #print(err + "\n")
            try:
                self.label_progress['text'] = "文字数が多すぎて表示できません。"
            except Exception:
                # Tkinter強制終了エラー回避(保険)
                pass

    # 終了処理
    def end(self, text_end="", flag_dt=False, flag_timer=False):
        self.i = 0
        if text_end == "":
            text_end = "再度開始できます..."
        dt_now = dt.datetime.now()  # 現在日時
        self.label_progress['text'] = "{0}{1}{2}".format(text_end, "【終了日時: {0}/{1}/{2} | {3}:{4}:{5}】".format(dt_now.year, dt_now.month if dt_now.month >= 10 else "0" + str(dt_now.month), dt_now.day if dt_now.day >= 10 else "0" + str(dt_now.day), dt_now.hour if dt_now.hour >= 10 else "0" + str(dt_now.hour), dt_now.minute if dt_now.minute >= 10 else "0" + str(dt_now.minute), dt_now.second if dt_now.second >= 10 else "0" + str(dt_now.second)) if flag_dt else "", "【動作時間: {}s】".format(time.time() - self.start) if flag_timer else "")

# 変化ラベル
class MoveProgressLabel:
    def __init__(self, place_x, place_y, text_len=10, text_ready=""):
        if text_ready == "":
            text_ready = "   Ready.   "
        self.flag_loop = False          # ループフラグ
        self.text_len = text_len
        self.yl = yajirushi_left()
        self.yr = yajirushi_right()
        self.set = ProgressLabel(place_x, place_y, next(self.yl) * self.text_len + text_ready + next(self.yr) * self.text_len)

    # 終了処理
    def end(self, text_end=""):
        if text_end == "":
            text_end = "   Finish   "
        self.yl = yajirushi_left()
        self.yr = yajirushi_right()
        self.set.end(next(self.yl) * self.text_len + text_end + next(self.yr) * self.text_len)

    # プログレススレッドスタート
    def progress_start(self, text_update):
        while self.flag_loop:
            self.set.update(next(self.yl) * self.text_len + text_update + next(self.yr) * self.text_len)
            time.sleep(0.1)

    # スレッドスタート
    def start(self, text_update=""):
        if text_update == "":
            text_update = " Processing "
        self.thread_progress = threading.Thread(target = self.progress_start, args=(text_update,))
        self.thread_progress.setDaemon(True)
        self.thread_progress.start()
    
# 矢印ジェネレータ左
def yajirushi_left():
    i = 0
    while 1:
        yield ">>>=<<<="[i % 8]
        i += 1

# 矢印ジェネレータ右
def yajirushi_right():
    i = 0
    while 1:
        yield "<<<=>>>="[i % 8]
        i += 1
        
#####################################################################
"""
    example
    備考: 進捗テスト

"""
# ↓↓↓直接このファイルを実行したときの処理
if __name__ == "__main__":
    #from fwmodule_gui_progress import *

    """
    # ウィジェット配置デバッグ用
    def debug_conf(ev):
        print(ev)
    """

    """
        GUIテスト(非確定的処理)
    """
    ################################################## 
    # 処理ターゲット
    class ProcessingTarget:
        # コンストラクタ
        def __init__(self, place_x, place_y, num):
            self.receiver = ProgressReceiver(place_x, place_y)
            self.label_progress = ProgressLabel(place_x+80, place_y-25)
            self.num = num      # ターゲット初期値

        # ターゲット
        def target(self):
            # ループするターゲットの処理
            while self.receiver.flag_loop:
                print("num : ", self.num)
                #time.sleep(1)          # 1秒もあるとスレッド重複処理が回避できない
                # なので分ける↓
                for i in range(5):      # 処理が重くなるので実質1.1秒くらい？
                    if self.receiver.flag_loop:
                        time.sleep(0.2)
                    else:
                        break

            # ループしないターゲットの処理
            #
            # ※処理内容※
            #
            self.receiver.flag_loop = False
            # 上記を最後に差し込む
        
        # スレッドスタート
        def start(self):
            self.thread_target = threading.Thread(target = self.target)     # 次開始で上書き
            self.thread_target.setDaemon(True)                              # デーモンスレッドに設定
            self.thread_target.start()                                      # 処理開始
            # デーモン化することでメインスレッド(Tkinter)が強制終了するとサブスレッド(thread_target)は自動的に破棄される。
            # (thread_targetに紐づいているrecieverも同様に破棄)

    # GUIアプリケーション
    class GuiApplication(tk.Frame):
        # コンストラクタ
        def __init__(self, master=None):
            window_width = 630
            window_height = 500

            # Frameクラスを継承
            super().__init__(master, width=window_width, height=window_height)
            
            # 初期値代入
            self.master = master
            self.master.title("GUIテスト(非確定的処理)")
            self.master.minsize(window_width, window_height)
            self.pack()
            #self.bind('<Configure>', debug_conf)

            # 処理ターゲット
            self.target = ProcessingTarget(place_x=350, place_y=50, num=1)

            # ウィジェット作成
            self.create_widgets()

        # ウィジェット作成
        def create_widgets(self):
            # ボタン作成
            self.button_start = tk.ttk.Button(self, text="開始", padding=10, command=self.start_event)
            self.button_start.place(x=100, y=100)
            self.button_change = tk.ttk.Button(self, text="変換", padding=10, command=self.change_event)
            self.button_change.place(x=100, y=225)
            self.button_end = tk.ttk.Button(self, text="終了", padding=10, command=self.end_event)
            self.button_end.place(x=100, y=350)

        # 処理開始
        def start_event(self):
            if not self.target.receiver.flag_loop:      # スレッド重複処理の防止
                print("開始しました")
                self.target.receiver.flag_loop = True
                self.target.receiver.flag_progress = False
                self.target.receiver.start()
                self.target.start()
                self.target.label_progress.update("num : {}".format(self.target.num))

        # 接続変更
        def change_event(self):
            self.target.num += 1
            self.target.label_progress.update("num : {}".format(self.target.num))

        # 処理終了
        def end_event(self):
            if self.target.receiver.flag_loop:
                self.target.receiver.flag_loop = False          # 停止(スレッド破棄)
                self.target.receiver.flag_progress = False      # 変化ラベルリセット
                self.target.label_progress.end()
                print("終了しました")

        # 強制終了通知
        def thread_end(self):
            self.target.receiver.flag_loop = False          # 停止(スレッド破棄)
            # 処理途中でTkinterを閉じたときにデーモンスレッドも一緒に破棄されるはずだが、
            # Tkinterを閉じても次のTkinterが開けばメインスレッドは持続されていることとなり、
            # デーモンスレッドは破棄されず残ったまま処理が続行されてしまうので、
            # スレッドフラグをOFFにして明示的にスレッドを終了させる。
            print("強制終了しました")
    
    # アプリケーション起動
    print("GUI1 起動...")
    window = tk.Tk()
    app1 = GuiApplication(master=window)
    app1.mainloop()
    print("GUI1 終了...")

    # スレッド強制終了の場合
    if app1.target.receiver.flag_loop:
        app1.thread_end()

    """
        GUIテスト(確定的処理)
    """
    ################################################## 
    # 処理ターゲット
    class ProcessingTarget:
        # コンストラクタ
        def __init__(self, self_root, bar_x, bar_y, bar_len):
            self.flag_running = False       # 処理中フラグ
            self.progressbar = Progressbar(self_root, bar_x, bar_y, bar_len)

        # ターゲット
        def target(self):
            print("開始しました")
            self.flag_running = True

            # 処理内容
            ##############################################
            len = 200
            self.progressbar.set.configure(maximum=len)
            for i in range(len):
                self.progressbar.update(i)
                time.sleep(0.01)
            ##############################################
            
            self.flag_running = False
            print("終了しました")
        
        # スレッドスタート
        def start(self):
            self.thread_target = threading.Thread(target = self.target)     # 次開始で上書き
            self.thread_target.setDaemon(True)                              # デーモンスレッドに設定
            self.thread_target.start()                                      # 処理開始
            # デーモン化することでメインスレッド(Tkinter)が強制終了するとサブスレッド(thread_target)は自動的に破棄される。

    # GUIアプリケーション
    class GuiApplication(tk.Frame):
        # コンストラクタ
        def __init__(self, master=None):
            window_width = 335
            window_height = 150

            # Frameクラスを継承
            super().__init__(master, width=window_width, height=window_height)
            
            # 初期値代入
            self.master = master
            self.master.title("GUIテスト(確定的処理)")
            self.master.minsize(window_width, window_height)
            self.pack()
            #self.bind('<Configure>', debug_conf)

            # 処理ターゲット
            self.target = ProcessingTarget(self, bar_x=10, bar_y=30, bar_len=315)

            # ウィジェット作成
            self.create_widgets()

        # ウィジェット作成
        def create_widgets(self):
            # ボタン作成
            self.start_button = tk.ttk.Button(self, command=self.start_event, text="開始")
            self.start_button.place(x=60, y=80)
            self.reset_button = tk.ttk.Button(self, command=self.reset_event, text="リセット")
            self.reset_button.place(x=195, y=80)

        # 処理開始
        def start_event(self):
            if not self.target.flag_running:    # スレッド重複処理の防止
                self.target.start()

        # リセット
        def reset_event(self):
            self.target.progressbar.reset()     # プログレスバー処理終了でリセット可能

        # 強制終了通知
        def thread_end(self):
            # 処理途中でTkinterを閉じたときにデーモンスレッドも一緒に破棄される。
            print("強制終了しました")
    
    # アプリケーション起動
    print("GUI起動...")
    window = tk.Tk()
    app2 = GuiApplication(master=window)
    app2.mainloop()
    print("GUI終了...")

    # スレッド強制終了の場合
    if app2.target.flag_running:
        app2.thread_end()

    # シャットダウン
    #import sys
    #sys.exit()
    