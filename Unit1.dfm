object Form1: TForm1
  Left = 0
  Top = 0
  Caption = 'by93Crawler'
  ClientHeight = 569
  ClientWidth = 941
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesktopCenter
  OnConstrainedResize = FormConstrainedResize
  OnCreate = FormCreate
  DesignSize = (
    941
    569)
  PixelsPerInch = 96
  TextHeight = 13
  object Edit1: TEdit
    Left = 8
    Top = 7
    Width = 201
    Height = 21
    TabOrder = 0
    TextHint = 'Account'
  end
  object Edit2: TEdit
    Left = 8
    Top = 34
    Width = 201
    Height = 21
    TabOrder = 1
    TextHint = 'Password'
  end
  object Button1: TButton
    Left = 215
    Top = 7
    Width = 66
    Height = 48
    Caption = 'Login'
    TabOrder = 2
    OnClick = Button1Click
  end
  object Memo1: TMemo
    Left = 8
    Top = 61
    Width = 925
    Height = 474
    Anchors = [akLeft, akTop, akRight, akBottom]
    ScrollBars = ssBoth
    TabOrder = 3
  end
  object Button2: TButton
    Left = 837
    Top = 7
    Width = 96
    Height = 48
    Anchors = [akTop, akRight]
    Caption = 'Show Response'
    Default = True
    TabOrder = 4
    OnClick = Button2Click
  end
  object Button3: TButton
    Left = 300
    Top = 8
    Width = 46
    Height = 47
    Caption = 'Start'
    TabOrder = 5
    OnClick = Button3Click
  end
  object Button4: TButton
    Left = 352
    Top = 8
    Width = 45
    Height = 47
    Caption = 'End'
    TabOrder = 6
    OnClick = Button4Click
  end
  object StatusBar1: TStatusBar
    Left = 0
    Top = 550
    Width = 941
    Height = 19
    Panels = <
      item
        Width = 200
      end
      item
        Width = 200
      end>
  end
  object Button5: TButton
    Left = 772
    Top = 8
    Width = 59
    Height = 47
    Anchors = [akTop, akRight]
    Caption = 'Census'
    TabOrder = 8
    OnClick = Button5Click
  end
  object Button6: TButton
    Left = 439
    Top = 8
    Width = 65
    Height = 47
    Caption = 'Extracting'
    TabOrder = 9
    OnClick = Button6Click
  end
  object Button7: TButton
    Left = 510
    Top = 8
    Width = 75
    Height = 47
    Caption = 'Extracting '#916
    TabOrder = 10
    OnClick = Button7Click
  end
end
