"""fwdef.py"""

import os
import sys
import time
from tqdm.autonotebook import tqdm  # Jupyter Notebook 対応
import subprocess
import ctypes
import _winapi
ctypes.windll.kernel32.SetStdHandle(_winapi.STD_INPUT_HANDLE, 0)    # subprocessをexe化で利用できるためのバグ回避コード
# subprocess.py内の1117行の以下、
# p2cread = _winapi.GetStdHandle(_winapi.STD_INPUT_HANDLE)
# が悪さをしています。
# この問題をより正確に言うと、PyInstaller で --noconsole かつ --onefile で作成したファイルを実行すると、
# 何故か GetStdHandle() API が INVALID_HANDLE_VALUE を返すため、
# subprocess にある、現在のプロセスの標準入出力ハンドルを取得しようとするデフォルトの処理が失敗し例外を生じてしまう。
# なので、次のように SetStdHandle() API を用い、とりあえず自身の標準入力ハンドルを NULL で上書きしてしまうことでも、今回の問題は回避可能。
# 標準ストリームのハンドルが異常値となることで不具合を招く可能性は、 subprocess 以外のモジュールでも考えられるため、
# 万が一そのような罠を踏んでしまった場合は、こちらの方がより汎用的で楽な解決方法になる。

#####################################################################
"""
    関数ネイキッドデコレータ
    備考: 関数詳細を表示するデバッグ用デコレータ

"""
def func_naked(func):
    import functools
    @functools.wraps(func)
    def wrapper(*args, **kwargs):   # どんな引数でも可能な可変長引数
        ret = func(*args, **kwargs)                     # 関数実行
        print("Funcname  : {}".format(func.__name__))   # 関数名
        print("Arguments : {}".format(args))            # タプル(リスト)型引数
        print("Keywords  : {}".format(kwargs))          # 辞書型引数
        print("Return    : {}".format(ret))             # 戻り値
        return ret
    return wrapper

#####################################################################
"""
    テスト

"""
# ↓↓↓直接このファイルを実行したときの処理
if __name__ == "__main__":
    #from fwdef import *

    @func_naked
    def func(msg1, msg2, flag=False, mode=3):
        print(msg1 + " " + msg2)
        return 123456
    
    # 実行
    result = func("Hello", "World!", flag=True)
    print(result)

    input(">>>\n>>>\n>>>\n処理が終了しました。\n")
    