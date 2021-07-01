"""module_bit.py"""

#####################################################################
"""
    2進数、8進数、10進数、16進数の相互変換
    備考:
        ・baseへの入力はradixに乗っ取った変換したいものでint型(10進数以外では0b, 0o, 0x表記必須)
        ・radix(基数)とoperand(変換対象)への入力はint型(初期値: radix=16進数 operand=2進数)
        ・flag_printがTrueの場合print出力する
        ・print出力結果は進数表記となる(戻り値は文字列なのでintでキャストする)
    ex) number_conv(base=0b10100, radix=2, operand=16) = 0x14

"""
def number_conv(base, radix=16, operand=2, flag_print=False):
    if radix == 10:
        if operand == 2:
            num = bin(base)
            if flag_print:
                print("10進数→2進数: " + num)
            return int(num, 0)
        elif operand == 8:
            num = oct(base)
            if flag_print:
                print("10進数→8進数: " + num)
            return int(num, 0)
        elif operand == 16:
            num = hex(base)
            if flag_print:
                print("10進数→16進数: " + num)
            return int(num, 0)
        else:
            if flag_print:
                print("オペランドが間違っています。\nやり直してください。")
    elif radix == 2:
        if operand == 10:
            num = base
            if flag_print:
                print("2進数→10進数: " + "{}".format(num))
            return num
        elif operand == 8:
            num = oct(base)
            if flag_print:
                print("2進数→8進数: " + "{}".format(num))
            return int(num, 0)
        elif operand == 16:
            num = hex(base)
            if flag_print:
                print("2進数→16進数: " + "{}".format(num))
            return int(num, 0)
        else:
            if flag_print:
                print("オペランドが間違っています。\nやり直してください。")
    elif radix == 8:
        if operand == 10:
            num = base
            if flag_print:
                print("8進数→10進数: " + "{}".format(num))
            return num
        elif operand == 2:
            num = bin(base)
            if flag_print:
                print("8進数→2進数: " + "{}".format(num))
            return int(num, 0)
        elif operand == 16:
            num = hex(base)
            if flag_print:
                print("8進数→16進数: " + "{}".format(num))
            return int(num, 0)
        else:
            if flag_print:
                print("オペランドが間違っています。\nやり直してください。")
    elif radix == 16:
        if operand == 10:
            num = base
            if flag_print:
                print("16進数→10進数: " + "{}".format(num))
            return num
        elif operand == 2:
            num = bin(base)
            if flag_print:
                print("16進数→2進数: " + "{}".format(num))
            return int(num, 0)
        elif operand == 8:
            num = oct(base)
            if flag_print:
                print("16進数→8進数: " + "{}".format(num))
            return int(num, 0)
        else:
            if flag_print:
                print("オペランドが間違っています。\nやり直してください。")
    else:
        if flag_print:
            print("基数が間違っています。\nやり直してください。")

#####################################################################
"""
    ビットシフト
    備考:
        ・base(基数)への入力はradixに乗っ取ったシフトしたいものでint型(10進数以外では0b, 0o, 0x表記必須)
        ・shiftへの入力はint型(いくつシフトするか)
        ・radix(基数)への入力はbaseで入力する進数型でint型(初期値: 2進数)
        ・lrへの入力は"left"か"right"のstr型(初期値: 右シフト)
        ・flag_printがTrueの場合print出力する
        ・print出力結果は2進数表記となる(戻り値は文字列なのでintでキャストする)
    ex) bit_shift(base=0b10100, shift=3) = 0b10

"""
def bit_shift(base, shift, radix=2, lr="right", flag_print=False):
    if radix == 10:
        num0 = bin(base)
        if lr == "left":
            num1 = bin(base << shift)
            if flag_print:
                print("シフト基数(10進数):   " + num0)
                print("10進数左シフト(結果): " + num1)
            return int(num1, 0)
        elif lr == "right":
            num1 = bin(base >> shift)
            if flag_print:
                print("シフト基数(10進数):   " + num0)
                print("10進数右シフト(結果): " + num1)
            return int(num1, 0)
        else:
            if flag_print:
                print("左シフトか右シフトかの指定がありません。\nやり直してください。")
    elif radix == 2:
        num0 = bin(base)
        if lr == "left":
            num1 = bin(base << shift)
            if flag_print:
                print("シフト基数(2進数):   " + "{}".format(num0))
                print("2進数左シフト(結果): " + "{}".format(num1))
            return int(num1, 0)
        elif lr == "right":
            num1 = bin(base >> shift)
            if flag_print:
                print("シフト基数(2進数):   " + "{}".format(num0))
                print("2進数右シフト(結果): " + "{}".format(num1))
            return int(num1, 0)
        else:
            if flag_print:
                print("左シフトか右シフトかの指定がありません。\nやり直してください。")
    elif radix == 8:
        num0 = bin(base)
        if lr == "left":
            num1 = bin(base << shift)
            if flag_print:
                print("シフト基数(8進数):   " + "{}".format(num0))
                print("8進数左シフト(結果): " + "{}".format(num1))
            return int(num1, 0)
        elif lr == "right":
            num1 = bin(base >> shift)
            if flag_print:
                print("シフト基数(8進数):   " + "{}".format(num0))
                print("8進数右シフト(結果): " + "{}".format(num1))
            return int(num1, 0)
        else:
            if flag_print:
                print("左シフトか右シフトかの指定がありません。\nやり直してください。")
    elif radix == 16:
        num0 = bin(base)
        if lr == "left":
            num1 = bin(base << shift)
            if flag_print:
                print("シフト基数(16進数):   " + "{}".format(num0))
                print("16進数左シフト(結果): " + "{}".format(num1))
            return int(num1, 0)
        elif lr == "right":
            num1 = bin(base >> shift)
            if flag_print:
                print("シフト基数(16進数):   " + "{}".format(num0))
                print("16進数右シフト(結果): " + "{}".format(num1))
            return int(num1, 0)
        else:
            if flag_print:
                print("左シフトか右シフトかの指定がありません。\nやり直してください。")
    else:
        if flag_print:
            print("基数が間違っています。\nやり直してください。")
        
#####################################################################
"""
    ビットマスク
    備考:
        ・base(基数)とmask(マスク)への入力は自由でどの進数もint型(10進数はそのままでその他の進数は0b, 0o, 0x表記)
        ・operandへの入力はmaskに乗っ取ったマスクする進数でint型(operandは2進数か16進数しか選べない)(初期値: 16進数)
        ・flag_printがTrueの場合print出力する
        ・print出力結果は入力したoperand(2進数か16進数)の表記となる(戻り値は文字列なのでintでキャストする)
    ex) bit_mask(base=0x01000001, mask=0x000000FF) = 0x1
        bit_mask(base=bit_shift(0x01000001, 24, 16), mask=0x000000FF) = 0x1

"""
def bit_mask(base, mask, operand=16, flag_print=False):
    if operand == 2:
        num0 = bin(base)
        num1 = bin(mask)
        num3 = bin(base & mask)
        if flag_print:
            print("マスク基数(2進数): " + num0)
            print("マスク(2進数)    : " + num1)
            print("2進数マスク(結果): " + "{}".format(num3))
        return int(num3, 0)
    elif operand == 16:
        num0 = hex(base)
        num1 = hex(mask)
        num3 = hex(base & mask)
        if flag_print:
            print("マスク基数(16進数): " + num0)
            print("マスク(16進数)    : " + num1)
            print("16進数マスク(結果): " + "{}".format(num3))
        return int(num3, 0)
    else:
        if flag_print:
            print("2進数か16進数かのマスク指定がありません。\nやり直してください。")

#####################################################################
"""
    テスト

"""
# ↓↓↓直接このファイルを実行したときの処理
if __name__ == "__main__":
    #from module_bit import *

    #--------進数相互変換--------#
    print("●2進数、8進数、10進数、16進数の相互変換")
    print(number_conv(0b10100, 2, 16, flag_print=True))
    print()
    
    #--------ビット操作--------#
    # ビットシフト
    print("●ビットシフト")
    print(bit_shift(0b10100, 3, 2, "right", flag_print=True))
    print()
    
    # ビットマスク
    print("●ビットマスク")
    print(bit_mask(0x01000001, 0x000000FF, 16, flag_print=True))
    print()

    # シフト＆マスク
    print("●シフト＆マスク")
    print(bit_mask(bit_shift(0x01000001, 24, 16, "right", flag_print=True), 0x000000FF, 16, flag_print=True))
    print()
    
    input(">>>\n>>>\n>>>\n処理が終了しました。\n")
    