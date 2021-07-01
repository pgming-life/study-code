"""fwmodule_console_progress.py"""

flag_release = False

try:
    import pgming_package.release as r
except Exception:
    # エラースルー
    pass
else:
    flag_release = r.flag

# 上記のインポートが読み込めない場合はflag_releaseは基本的にFalse
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
    pythonスタート

"""
def python_start():
    print("\n本当に実行させますか？")
    print("このまま実行するにはEnterキーを押してください...")
    print("    (  I        I  )")
    print("    (  I   II   I  )")
    print("   ((  I  I  I  I  ))")
    print("  ((  I   I  I   I  ))")
    print(" ((  I   I    I   I  ))")
    print(" ((  I    I  I    I  ))")
    print(" ( (  I   I  I   I  ) )")
    print(" ((( I I   II   I I )))")
    print("  (((I  I  II  I  I)))")
    print("   ([0)  I    I  (0])")
    print("    ((I   I  I   I))")
    print("     ((I        I))")
    print("      ()I      I()")
    print("       ( o    o )")
    input("        (__人__)")
    print("           ((")
    print("            ))")
    print("           ((")

#####################################################################
"""
    繋ぎ
    備考: 計算結果を出力するときの繋ぎ

"""
def python_connection():
    print("            ))")
    print("           ((")
    print("            ))")
    print("           ((")

#####################################################################
"""
    サブ計算(非確定的処理)
    備考: サブ計算を使うのはrangeの最大値が決まっていない(tqdmは使えない)とき。だからグルグル棒で表す。

"""
class python_sub:
    # コンストラクタ
    def __init__(self, name_calc):
        # インスタンス変数
        self.name_calc = name_calc
        self.g = guruguru()

    # # インスタンスメソッド
    # スタート(for文が始まる前の外側に挿入)
    def start(self):
        print("{}...\r".format(self.name_calc), end="")
    
    # 計算中(for文中に挿入)
    def calc(self):
        # クラス変数
        print("{0}...{1}\r".format(self.name_calc, next(self.g)), end="")

    # 終了処理
    def end(self):
        print("{}...OK!!".format(self.name_calc))

# グルグル棒ジェネレータ
def guruguru():
    i = 0
    while 1:
        yield "|/--/"[i % 5]
        i += 1

#####################################################################
"""
    メイン計算(確定的処理)
    備考: メイン計算ではtqdmを扱う

"""
#for i in tqdm(range(?)):      # for文にtqdmを掛ける

#####################################################################
"""
    pythonエンド

"""
def python_end():
    print("            ))")
    print("           ((")
    print("            ))")
    print("           ((")
    print("            ))")
    print("           ((")
    print("           )(")
    input("\n処理が終了しました。ウィンドウを閉じるにはEnterを押してください...\n\n")
    sys.exit()

#####################################################################
"""
    クラス型デコレータ
    備考:
    ・コンストラクタとデストラクタの機能があり、それぞれPythonスタートとPythonエンドを持つ
    ・メイン関数に装飾することで使用可能

"""
def class_decolater(func):
    def wrapper():
        python_start()  # Pythonスタート
        func()          # メイン関数実行
        python_end()    # Pythonエンド
    return wrapper

#####################################################################
"""
    example
    備考: 進捗テスト

"""
# ↓↓↓直接このファイルを実行したときの処理
if __name__ == "__main__":
    #from fwmodule_console_progress import *

    # メイン関数
    @class_decolater
    def main_func():
        # 繋ぎ
        for i in range(5):
            print("出力結果の表示{}".format(i))
        python_connection()
        for i in range(5):
            print("出力結果の表示{}".format(i))
        python_connection()

        # サブ計算(非確定的処理)
        sub = python_sub("サブ計算")
        sub.start()
        for i in range(100):
            sub.calc()
            time.sleep(.01)
        sub.end()
        python_connection()

        # メイン計算(確定的処理)
        print("メイン計算")
        for i in tqdm(range(100)):
            time.sleep(.01)
        python_connection()

        # 自作プログレスバー
        for i in range(1, 21):
            sys.stdout.write("\r[ %-20s %3.0f%% ]" % ("#" * i, i * 100 / 20))
            sys.stdout.flush()
            time.sleep(.1)
        print()
        python_connection()

        # 自作プログレスキャラ
        for i in range(1, 51):
            char = "|>-<"[i % 4] 
            sys.stdout.write("\r%s %3.0f%% done %s" % (char * 10, i * 100 / 50, char * 10))
            sys.stdout.flush()
            time.sleep(.1)
        print()

    # メイン関数実行
    main_func()

    # シャットダウン
    #import sys
    #sys.exit()
    