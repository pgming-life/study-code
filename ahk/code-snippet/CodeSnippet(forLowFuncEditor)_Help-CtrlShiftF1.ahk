#IfWinActive

;�O���[�o���ϐ�
global g_indent_space := 
global g_switch := 
global g_case_num := 
global g_break := 0
global g_name := 
global g_public := 1
global g_private := 1
global g_protected := 0
global g_switch_flag := False
global g_class_flag := False
global g_active_id := 
global g_tab :=
global g_tab_space := 

;�w���v
^+F1::
    WinGet, active_id, ID, A
    MsgBox, ���w���v��`n`n------------------------------------------------------------------------------------------`n`n�Eif`nCtrl + Shift + I`n`n�Eelse if`nCtrl + Shift + Alt + E`n`n�Eelse`nCtrl + Shift + E`n`n�Eswitch`nCtrl + Shift + Alt + S`n`n�Efor`nCtrl + Shift + F`n`n�Ewhile`nCtrl + Shift + W`n`n�Estruct`nCtrl + Shift + T`n`n�Eenum`nCtrl + Shift + Y`n`n�Eclass`nCtrl + Shift + U`n`n------------------------------------------------------------------------------------------`n`n�E�C���f���g�X�y�[�X�̓o�^`nCtrl + Shift + N`n`n�E�C���f���g�̏o��`nCtrl + Shift + Enter`n`n��`n�u�C���f���g�X�y�[�X�̓o�^�v�́A�uShift + Home�v�̃L�[�𑗐M���擪�X�y�[�X���R�s�[����B`n����ɂ��A�u�C���f���g�̏o�́v�ŉ��s���Ă���R�s�[�����擪�X�y�[�X���y�[�X�g����B`n`n------------------------------------------------------------------------------------------`n`n�E�擪�ɂP�̃^�u��ǉ�`nCtrl + Shift + J`n`n�E�P�̃^�u�̃X�y�[�X����o�^`nCtrl + Shift + Alt + K`n`n�E�擪�̂P�̃^�u�X�y�[�X���폜`nCtrl + Shift + K`n`n��`n�^�u�X�y�[�X�����J��K�̃z�b�g�L�[�̓R�s�y�ɂ��C���f���g�X�y�[�X�̒����ɗp���A`n�����Ċe�s�Œ����ł���悤�ɏ����̍Ō�ɁuDown�v�ƁuHome�v�̃L�[�𑗐M���Ă���B`n�������⑁���A�ł͌��ցB`n`n------------------------------------------------------------------------------------------`n`n�E�����r����~`nF8������
    WinActivate, ahk_id %active_id%
    Return

;if��
^+I::
    ;���݂̃A�N�e�B�u�E�B���h�E��ID���擾
    WinGet, active_id, ID, A
    
    ;������`���ʓ��͊֐�
    def := InputDefine(active_id)
    If(!def)
        Return
    
    ;F8�������ŏ�����~
    If(!CancelProcess(False))
        Return
    
    ;�ŏ��Ɏ擾����ID�ɊY������E�B���h�E���A�N�e�B�u
    WinActivate, ahk_id %active_id%
    ;�C���v�b�g�_�C�A���O�{�b�N�X������Ƃ��ɁA
    ;�Ӑ}�������̃E�B���h�E���A�N�e�B�u�ɂȂ邱�Ƃ�h�����߁B
    
    If(!CancelProcess(False))
        Return
    
    ;���p�p�����ɐ؂�ւ�
    IMEGetState()
    
    If(!CancelProcess(False))
        Return
    
    ;�N���b�v�{�[�h�̃o�b�N�A�b�v
    backup_cb = %ClipboardAll%
    
    ;�N���b�v�{�[�h���N���A
    Clipboard = 
    ;�N���b�v������̂��󂾂ƁA
    ;�o�b�N�A�b�v����Ă���N���b�v���Q�Ƃ��Ă��܂����߁A
    ;�N���b�v�{�[�h���N���A����B
    
    ;�������o�́�����
    ;�\�����ʏo��
    If(!SyntaxCommon(active_id, backup_cb, "if(", def))
        Return
    
    ;�o�b�N�A�b�v���N���b�v�{�[�h�ɖ߂�
    Clipboard = %backup_cb%
    
    Return

;else if
^+!E::
    WinGet, active_id, ID, A
    def := InputDefine(active_id)
    If(!def)
        Return
    If(!CancelProcess(False))
        Return
    WinActivate, ahk_id %active_id%
    If(!CancelProcess(False))
        Return
    IMEGetState()
    If(!CancelProcess(False))
        Return
    backup_cb = %ClipboardAll%
    Clipboard = 
    
    ;�������o�́�����
    If(!SyntaxCommon(active_id, backup_cb, "else if(", def))
        Return
    
    Clipboard = %backup_cb%
    Return

;else
^+E::
    WinGet, active_id, ID, A
    If(!CancelProcess(False))
        Return
    IMEGetState()
    If(!CancelProcess(False))
        Return
    backup_cb = %ClipboardAll%
    Clipboard = 
    
    ;�������o�́�����
    If(!SyntaxCommon(active_id, backup_cb, "else"))
        Return
    
    Clipboard = %backup_cb%
    Return

;switch��
^+!S::
    WinGet, active_id, ID, A
    g_active_id := active_id
    g_switch_flag := True
    g_class_flag := False
    Gui, Add, Text
    Gui, Add, Text, , switch����ϐ�����͂��Ă��������B
    Gui, Add, Edit, W200 vg_switch, val
    Gui, Add, Text
    Gui, Add, Text, , case�̐�����͂��Ă��������B
    Gui, Add, Edit, W200 vg_case_num, 3
    Gui, Add, Text
    Gui, Add, Checkbox, xm vg_break, �ubreak;�v�̗L��
    If(g_break)
        GuiControl, , g_break, 1
    Gui, Add, Text
    Gui, Add, Button, W100 X25 Default , OK
    Gui, Add, Button, W100 X+0, Cancel
    Gui, Show
    Return

;for��
^+F::
    WinGet, active_id, ID, A
    def := InputDefine(active_id)
    If(!def)
        Return
    If(!CancelProcess(False))
        Return
    WinActivate, ahk_id %active_id%
    If(!CancelProcess(False))
        Return
    IMEGetState()
    If(!CancelProcess(False))
        Return
    backup_cb = %ClipboardAll%
    Clipboard = 
    
    ;�������o�́�����
    If(!SyntaxCommon(active_id, backup_cb, "for(", def))
        Return
    
    Clipboard = %backup_cb%
    Return

;while��
^+W::
    WinGet, active_id, ID, A
    def := InputDefine(active_id)
    If(!def)
        Return
    If(!CancelProcess(False))
        Return
    WinActivate, ahk_id %active_id%
    If(!CancelProcess(False))
        Return
    IMEGetState()
    If(!CancelProcess(False))
        Return
    backup_cb = %ClipboardAll%
    Clipboard = 
    
    ;�������o�́�����
    If(!SyntaxCommon(active_id, backup_cb, "while(", def))
        Return
    
    Clipboard = %backup_cb%
    Return

;�\����
^+T::
    WinGet, active_id, ID, A
    Loop
    {
        InputBox, name, , �\���̖�����͂��Ă��������B, , , , , , , , StructName
        If(ErrorLevel <> 0)
        {
            WinActivate, ahk_id %id%
            Return
        }
        If(name == "" || RegExMatch(name, "\W"))
        {
            MsgBox, �P��\������([0-9a-zA-Z_])�̍\���̖�����͂��Ă��������B
        }
        Else
        {
            Break
        }
    }
    If(!CancelProcess(False))
        Return
    WinActivate, ahk_id %active_id%
    If(!CancelProcess(False))
        Return
    IMEGetState()
    If(!CancelProcess(False))
        Return
    backup_cb = %ClipboardAll%
    Clipboard = 
    
    ;�������o�́�����
    If(!SyntaxCommon(active_id, backup_cb, "struct", name))
        Return
    
    Clipboard = %backup_cb%
    Return

;�񋓑�
^+Y::
    WinGet, active_id, ID, A
    Loop
    {
        InputBox, name, , �񋓑̖�����͂��Ă��������B, , , , , , , , EnumName
        If(ErrorLevel <> 0)
        {
            WinActivate, ahk_id %id%
            Return
        }
        If(name == "" || RegExMatch(name, "\W"))
        {
            MsgBox, �P��\������([0-9a-zA-Z_])�̗񋓑̖�����͂��Ă��������B
        }
        Else
        {
            Break
        }
    }
    If(!CancelProcess(False))
        Return
    WinActivate, ahk_id %active_id%
    If(!CancelProcess(False))
        Return
    IMEGetState()
    If(!CancelProcess(False))
        Return
    backup_cb = %ClipboardAll%
    Clipboard = 
    
    ;�������o�́�����
    If(!SyntaxCommon(active_id, backup_cb, "enum", name))
        Return
    
    Clipboard = %backup_cb%
    Return

;�N���X
^+U::
    WinGet, active_id, ID, A
    g_active_id := active_id
    g_class_flag := True
    g_switch_flag := False
    Gui, Add, Text
    Gui, Add, Text, , �N���X������͂��Ă��������B
    Gui, Add, Edit, W200 vg_name, ClassName
    Gui, Add, Text
    Gui, Add, Checkbox, xm vg_public, �upublic:�v�̗L��
    Gui, Add, Checkbox, y+5 vg_private, �uprivate:�v�̗L��
    Gui, Add, Checkbox, y+5 vg_protected, �uprotected:�v�̗L��
    If(g_public)
        GuiControl, , g_public, 1
    If(g_private)
        GuiControl, , g_private, 1
    If(g_protected)
        GuiControl, , g_protected, 1
    Gui, Add, Text
    Gui, Add, Button, W100 X25 Default , OK
    Gui, Add, Button, W100 X+0, Cancel
    Gui, Show
    Return
    
;�C���f���g�X�y�[�X�̓o�^
^+!N::
    WinGet, active_id, ID, A
    backup_cb = %ClipboardAll%
    Clipboard = 
    If(!CancelProcess(True, cb))
        Return
    WinActivate, ahk_id %id%
    Send, +{Home}
    If(!CancelProcess(True, cb))
        Return
    WinActivate, ahk_id %id%
    Send, ^c
    If(!CancelProcess(True, cb))
        Return
    WinActivate, ahk_id %id%
    Send, {End}
    g_indent_space = %ClipboardAll%
    MsgBox, �C���f���g�X�y�[�X��o�^���܂����B
    WinActivate, ahk_id %id%
    Clipboard = %backup_cb%
    Return

;�C���f���g�̏o��
^+Enter::
    WinGet, active_id, ID, A
    If(!g_indent_space)
    {
        MsgBox, �C���f���g�X�y�[�X���o�^����Ă��܂���B`n�o�^���ĉ������B
        WinActivate, ahk_id %active_id%
        Return
    }
    backup_cb = %ClipboardAll%
    Clipboard = 
    If(!CancelProcess(True, backup_cb))
        Return
    WinActivate, ahk_id %active_id%
    Send, {Enter}
    If(!CancelProcess(True, backup_cb))
        Return
    WinActivate, ahk_id %active_id%
    Send, +{Home}
    If(!OutputString(active_id, backup_cb, g_indent_space))
        Return
    Clipboard = %backup_cb%
    Return

;�^�u�̏o�͉񐔂�o�^
^+!J::
    WinGet, active_id, ID, A
    Loop
    {
        InputBox, tab, , �^�u�̏o�͉񐔂���͂��Ă��������B
        If(ErrorLevel <> 0)
        {
            WinActivate, ahk_id %active_id%
            Return
        }
        ;�u���͂Ȃ��v���u���p�����ȊO��1�����v������������΂�蒼��
        If(tab == "" || RegExMatch(tab, "\D"))
        {
            MsgBox, ���p�̎��R������͂��Ă��������B
        }
        Else
        {
            Break
        }
    }
    g_tab := tab
    MsgBox, �^�u�̏o�͉񐔂�o�^���܂����B
    WinActivate, ahk_id %active_id%
    Return

;�擪�Ƀ^�u��ǉ�(���������⑁���A�ł͐���ɓ��삹���A�r���Łuj�v�����͂����)
^+J::
    If(!g_tab)
    {
        WinGet, active_id, ID, A
        Loop
        {
            InputBox, tab, , �^�u�̏o�͉񐔂���͂��Ă��������B
            If(ErrorLevel <> 0)
            {
                WinActivate, ahk_id %active_id%
                Return
            }
            ;�u���͂Ȃ��v���u���p�����ȊO��1�����v������������΂�蒼��
            If(tab == "" || RegExMatch(tab, "\D"))
            {
                MsgBox, ���p�̎��R������͂��Ă��������B
            }
            Else
            {
                Break
            }
        }
        g_tab := tab
        MsgBox, �^�u�̏o�͉񐔂�o�^���܂����B`n�����𑱍s���܂��B
        WinActivate, ahk_id %active_id%
    }
    Loop, %g_tab%
    {
        Send, {Tab}
    }
    Send, {Down}{Home}
    Return

;�P�̃^�u�̃X�y�[�X����o�^
^+!K::
    WinGet, active_id, ID, A
    Loop
    {
        InputBox, tab_space, , �P�̃^�u�̃X�y�[�X������͂��Ă��������B
        If(ErrorLevel <> 0)
        {
            WinActivate, ahk_id %active_id%
            Return
        }
        ;�u���͂Ȃ��v���u���p�����ȊO��1�����v������������΂�蒼��
        If(tab_space == "" || RegExMatch(tab_space, "\D"))
        {
            MsgBox, ���p�̎��R������͂��Ă��������B
        }
        Else
        {
            Break
        }
    }
    g_tab_space := tab_space
    MsgBox, �P�̃^�u�̃X�y�[�X����o�^���܂����B
    WinActivate, ahk_id %active_id%
    Return

;�擪�̂P�̃^�u�X�y�[�X���폜(���������⑁���A�ł͐���ɓ��삹���A�r���Łuk�v�����͂����)
^+K::
    If(!g_tab_space)
    {
        WinGet, active_id, ID, A
        Loop
        {
            InputBox, tab_space, , �P�̃^�u�̃X�y�[�X������͂��Ă��������B
            If(ErrorLevel <> 0)
            {
                WinActivate, ahk_id %active_id%
                Return
            }
            ;�u���͂Ȃ��v���u���p�����ȊO��1�����v������������΂�蒼��
            If(tab_space == "" || RegExMatch(tab_space, "\D"))
            {
                MsgBox, ���p�̎��R������͂��Ă��������B
            }
            Else
            {
                Break
            }
        }
        g_tab_space := tab_space
        MsgBox, �P�̃^�u�̃X�y�[�X����o�^���܂����B`n�����𑱍s���܂��B
        WinActivate, ahk_id %active_id%
    }
    Loop, %g_tab_space%
    {
        Send, {Delete}
    }
    Send, {Down}{Home}{Home}
    Return

;GUI�{�^������
ButtonOK:
    ;�ϐ��ɔ��f
    Gui, Submit
    
    ;GUI���
    Gui, Destroy
    
    ;switch��
    If(g_switch_flag)
    {
        ;�u���͂Ȃ��v���u�P��\������([0-9a-zA-Z_])�ȊO��1�����v������������΂�蒼��
        If(g_switch == "" || RegExMatch(g_switch, "\W"))
        {
            MsgBox, �P��\������([0-9a-zA-Z_])��switch�ϐ�����͂��Ă��������B
            
            ;�u���͂Ȃ��v���u���p�����ȊO��1�����v������������΂�蒼��
            If(g_case_num == "" || RegExMatch(g_case_num, "\D"))
                MsgBox, ���p�̎��R������͂��Ă��������B
            
            MsgBox, �z�b�g�L�[���������蒼���Ă��������B
            Return
        }
        
        If(g_case_num == "" || RegExMatch(g_case_num, "\D"))
        {
            MsgBox, ���p�̎��R������͂��Ă��������B
            MsgBox, �z�b�g�L�[���������蒼���Ă��������B
            Return
        }
        
        ;�v�f����̔z����쐬
        case_array := Object()
        
        i := 1
        Loop
        {
            ;���򂷂�ϐ��������ŃC���v�b�g
            InputBox, case%i%, , %i%�ڂ̕���ϐ�����͂��Ă��������B
            case_array.Insert(case%i%)
            If(ErrorLevel <> 0)
            {
                WinActivate, ahk_id %g_active_id%
                Return
            }
            
            ;�u���͂Ȃ��v���u�P��\������([0-9a-zA-Z_])�ȊO��1�����v������������΂�蒼��
            If(case%i% == "" || RegExMatch(case%i%, "\W"))
            {
                MsgBox, ���p�p��������͂��Ă��������B`n%i%�ڂɖ߂�܂��B
                Continue
            }
            
            If(i == g_case_num)
            {
                Break
            }
            
            i++
        }
        
        If(!CancelProcess(False))
            Return
        WinActivate, ahk_id %g_active_id%
        If(!CancelProcess(False))
            Return
        IMEGetState()
        If(!CancelProcess(False))
            Return
        backup_cb = %ClipboardAll%
        Clipboard = 
        
        ;�������o�́�����
        If(!SyntaxCommon(g_active_id, backup_cb, "switch(", g_switch))
            Return
        Clipboard = 
        If(!CancelProcess(True, backup_cb))
            Return
        WinActivate, ahk_id %g_active_id%
        Send, +{Home}
        If(!CancelProcess(True, backup_cb))
            Return
        WinActivate, ahk_id %g_active_id%
        Send, ^c
        front_clip = %ClipboardAll%
        If(!CancelProcess(True, backup_cb))
            Return
        WinActivate, ahk_id %g_active_id%
        Send, {End}
        c := "case"
        s := " "
        x := ":"
        b := "break`;"
        d := "default:"
        
        ;�C�e���[�^��case���o��(�C���f�b�N�X��1�Ŏn�܂�)
        For index, element in case_array
        {
            clip = %c%%s%%element%%x%
            If(!OutputString(g_active_id, backup_cb, clip))
                Return
            If(!CancelProcess(True, backup_cb))
                Return
            WinActivate, ahk_id %g_active_id%
            Send, {Enter}
            If(!CancelProcess(True, backup_cb))
                Return
            WinActivate, ahk_id %g_active_id%
            Send, +{Home}
            If(!OutputString(g_active_id, backup_cb, front_clip))
                Return
            If(g_break)
            {
                If(!CancelProcess(True, backup_cb))
                    Return
                WinActivate, ahk_id %g_active_id%
                Send, {Tab}
                If(!OutputString(g_active_id, backup_cb, b))
                    Return
                If(!CancelProcess(True, backup_cb))
                    Return
                WinActivate, ahk_id %g_active_id%
                Send, {Enter}
                If(!CancelProcess(True, backup_cb))
                    Return
                WinActivate, ahk_id %g_active_id%
                Send, +{Home}
                If(!OutputString(g_active_id, backup_cb, front_clip))
                    Return
            }
        }
        
        If(!OutputString(g_active_id, backup_cb, d))
            Return
        Loop, %g_case_num%
        {
            If(g_break)
            {
                If(!CancelProcess(True, backup_cb))
                    Return
                WinActivate, ahk_id %g_active_id%
                Send, {Up 2}
            }
            Else
            {
                If(!CancelProcess(True, backup_cb))
                    Return
                WinActivate, ahk_id %g_active_id%
                Send, {Up}
            }
        }
        If(!CancelProcess(True, backup_cb))
            Return
        WinActivate, ahk_id %g_active_id%
        Send, {End}
        If(!CancelProcess(True, backup_cb))
            Return
        WinActivate, ahk_id %g_active_id%
        Send, {Enter}
        If(!CancelProcess(True, backup_cb))
            Return
        WinActivate, ahk_id %g_active_id%
        Send, +{Home}
        If(!OutputString(g_active_id, backup_cb, front_clip))
            Return
        If(!CancelProcess(True, backup_cb))
            Return
        WinActivate, ahk_id %g_active_id%
        Send, {Tab}
        ;�������o�́�����
    }
    ;�N���X
    Else If(g_class_flag)
    {
        ;�u���͂Ȃ��v���u�P��\������([0-9a-zA-Z_])�ȊO��1�����v������������΂�蒼��
        If(g_name == "" || RegExMatch(g_name, "\W"))
        {
            MsgBox, �P��\������([0-9a-zA-Z_])�̃N���X������͂��Ă��������B
            MsgBox, �z�b�g�L�[���������蒼���Ă��������B
            Return
        }
        
        If(!CancelProcess(False))
            Return
        WinActivate, ahk_id %g_active_id%
        If(!CancelProcess(False))
            Return
        IMEGetState()
        If(!CancelProcess(False))
            Return
        backup_cb = %ClipboardAll%
        Clipboard = 
        
        ;�������o�́�����
        If(!SyntaxCommon(g_active_id, backup_cb, "class", g_name))
            Return
    }
    
    Clipboard = %backup_cb%
    Return
ButtonCancel:
GuiClose:
    Gui, Destroy
    Return

;if(else if)���Afor���Awhile���̏�����`���ʓ��͊֐�
InputDefine(id)
{
    Loop
    {
        ;�C���v�b�g
        InputBox, def, , ������`����͂��Ă��������B
        
        ;�L�����Z������
        If(ErrorLevel <> 0)
        {
            ;�_�C�A���O�{�b�N�X������珈���I��
            WinActivate, ahk_id %id%
            Return False
        }
        
        ;�u���͂Ȃ��v������������΂�蒼��
        If(def == "")
        {
            MsgBox, �������͂��Ă��������B
        }
        Else
        {
            Break
        }
    }
    
    Return def
}

;if(else if, else)���Aswitch���Afor���Awhile��[, struct, enum, class]�̍\�����ʏo�͊֐�
SyntaxCommon(id, cb, str, inputstr:="")
{
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, +{Home}
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, ^c
    front_clip = %ClipboardAll%
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, {End}
    s := " "
    b := (str == "else") ? "" : ")"
    If(str == "class" || str == "struct" || str == "enum")
    {
        clip = %str%%s%%inputstr%
    }
    Else
    {
        clip = %str%%inputstr%%b%
    }
    If(!OutputString(id, cb, clip))
        Return False
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, {Enter}
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, +{Home}
    If(!OutputString(id, cb, front_clip))
        Return False
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, +[
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, {Enter}
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, +{Home}
    If(!OutputString(id, cb, front_clip))
        Return False
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, {Enter}
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, +{Home}
    If(!OutputString(id, cb, front_clip))
        Return False
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, +]
    If(str == "struct" || str == "enum" || str == "class")
    {
        If(!CancelProcess(True, cb))
            Return False
        WinActivate, ahk_id %id%
        Send, {;}
        If(!CancelProcess(True, cb))
            Return False
        WinActivate, ahk_id %id%
        Send, {Left 2}
    }
    Else
    {
        If(!CancelProcess(True, cb))
            Return False
        WinActivate, ahk_id %id%
        Send, {Left}
    }
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, {Up}
    If(str == "class")
    {
        If(g_public)
        {
            clip = public:
            If(!OutputString(g_active_id, cb, clip))
                Return
            If(g_private || g_protected)
            {
                If(!CancelProcess(True, cb))
                    Return
                WinActivate, ahk_id %g_active_id%
                Send, {Enter}
                If(!CancelProcess(True, cb))
                    Return
                WinActivate, ahk_id %g_active_id%
                Send, +{Home}
                If(!OutputString(g_active_id, cb, front_clip))
                    Return
            }
        }
        If(g_private)
        {
            clip = private:
            If(!OutputString(g_active_id, cb, clip))
                Return
            If(g_protected)
            {
                If(!CancelProcess(True, cb))
                    Return
                WinActivate, ahk_id %g_active_id%
                Send, {Enter}
                If(!CancelProcess(True, cb))
                    Return
                WinActivate, ahk_id %g_active_id%
                Send, +{Home}
                If(!OutputString(g_active_id, cb, front_clip))
                    Return
            }
        }
        If(g_protected)
        {
            clip = protected:
            If(!OutputString(g_active_id, cb, clip))
                Return
        }
        If(g_public)
        {
            If(g_private && g_protected)
            {
                If(!CancelProcess(True, cb))
                    Return
                WinActivate, ahk_id %g_active_id%
                Send, {Up 2}
            }
            Else If(g_private || g_protected)
            {
                If(!CancelProcess(True, cb))
                    Return
                WinActivate, ahk_id %g_active_id%
                Send, {Up}
            }
        }
        Else
        {
            If(g_private && g_protected)
            {
                If(!CancelProcess(True, cb))
                    Return
                WinActivate, ahk_id %g_active_id%
                Send, {Up}
            }
        }
        If(g_public || g_private || g_protected)
        {
            If(!CancelProcess(True, cb))
                Return
            WinActivate, ahk_id %g_active_id%
            Send, {End}
            If(!CancelProcess(True, cb))
                Return
            WinActivate, ahk_id %g_active_id%
            Send, {Enter}
            If(!CancelProcess(True, cb))
                Return
            WinActivate, ahk_id %g_active_id%
            Send, +{Home}
            If(!OutputString(g_active_id, cb, front_clip))
                Return
        }
    }
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, {Tab}
    
    Return True
}

;������o�͊֐�
OutputString(id, cb, str)
{
    Clipboard = %str%
    
    ;�X���[�v0.15�b
    Sleep, 150
    ;�N���b�v�{�[�h�ɃR�s�[������ɂ����Ƀy�[�X�g����ƁA
    ;�������������Ĕ�΂���Ă��܂����f����Ȃ��̂ŃX���[�v������B
    
    If(!CancelProcess(True, cb))
        Return False
    WinActivate, ahk_id %id%
    Send, ^v
    
    Return True
}

;�L�[������������~�֐�
CancelProcess(cbflag, cb:="")
{
    ;F8�������Œ�~(Sleep���l�����čŒ��150�~���b��)
    GetKeyState, result, F8, P
    If(result == "D")
    {
        If(cbflag)
            Clipboard = %cb%
        MsgBox, �������~���܂����B
        Return False
    }
    
    Return True
}

;IME��Ԃ̎擾�֐�
IMEGetState()
{
    DetectHiddenWindows, On
    WinGet, current_window, ID, A
    get_default := DllCall("imm32.dll\ImmGetDefaultIMEWnd", "Uint", current_window)
    ime_state := DllCall("user32.dll\SendMessageA", "UInt", get_default, "UInt", 0x0283, "Int", 0x0005, "Int", 0)
    DetectHiddenWindows, Off
    If (ime_state)
        Send, {vkf3}
    ;IME��Ԃ�1��IME�I��(�S�p�Ђ炪��)
    ;IME��Ԃ�0��IME�I�t(���p�p����)
}

#IfWinActive
