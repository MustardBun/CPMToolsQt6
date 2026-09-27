#!/usr/bin/env python3
"""Generate Qt .ts translation catalogues for CpmtoolsGUI.

This project does not run `lupdate` in the build, because that requires the
qt6-declarative tooling (Qt6Qml.dll). Instead this script holds the catalogue
of translatable strings together with their translations, and writes standard
.ts files that `lrelease` compiles and embeds.

Run it whenever a UI string changes:

    python scripts/gen_translations.py

Copyright (C) 2026 Joseph Kwok (@MustardBun)
SPDX-License-Identifier: GPL-3.0-or-later
"""

from __future__ import annotations

import os
import xml.etree.ElementTree as ET
from xml.dom import minidom

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT_DIR = os.path.join(ROOT, "i18n")

# Languages we ship, in menu order. The value is the Qt language attribute.
LANGUAGES = [
    ("en", "en_US"),
    ("ja", "ja_JP"),
    ("zh_CN", "zh_CN"),
    ("zh_TW", "zh_TW"),
    ("it", "it_IT"),
    ("fr", "fr_FR"),
    ("de", "de_DE"),
    ("es", "es_ES"),
]

# Languages with a single plural form.
SINGLE_PLURAL = {"ja", "zh_CN", "zh_TW"}

# ---------------------------------------------------------------------------
# Plural entries are marked with the sentinel PLURAL.
# ---------------------------------------------------------------------------
PLURAL = "__plural__"


def plural(singular_en, plural_en):
    """A plural source string with its English forms."""
    return (PLURAL, singular_en, plural_en)


# ---------------------------------------------------------------------------
# The string catalogue.
#
#   "Context": [ source, (PLURAL, singular_en, plural_en), ... ]
#
# and TRANSLATIONS[lang][source] = translation. Plural entries use
# TRANSLATIONS[lang][source] = (singular, plural).
# ---------------------------------------------------------------------------

MAINWINDOW = [
    "CPMToolsQt6",
    "Drop a disk image here, or press Select...",
    "Disk image to inspect (.dsk, .ddi, .img)",
    "Select...",
    "Image file",
    "Format",
    "Files inside the image. Drag out to copy them to your desktop, "
    "or drop files here to add them.",
    "Show",
    "Files in image",
    "Filter",
    "Files on this computer",
    "Folder path - type or paste, or press Browse...",
    "Path of the folder shown below. A file path selects that file in its folder.",
    "Browse...",
    "Choose a folder, or a file inside a folder",
    "Browse this computer",
    "What would you like to open?",
    "Folder...",
    "File...",
    "Choose a folder",
    "Choose a file",
    "Folder not found",
    "There is no file or folder at:\n%1",
    "Showing %1",
    "Showing %1 (selected %2)",
    "&Get from image",
    "&Put into image",
    "&Delete",
    "Copy the selected image files to the folder shown on the right",
    "Copy the files selected on the right into the image",
    "Delete the selected files from the image",
    "&File",
    "&Open image...",
    "&New image...",
    "E&xit",
    "&Edit",
    "&Settings",
    "UI &Font Size",
    "&Small",
    "&Medium",
    "The system default size",
    "&Large",
    "&Extra large",
    "&Language",
    "&System default",
    "&Help",
    "On&line",
    "&About",
    "Main",
    "Ready",
    "Language",
    'The translation for "%1" could not be loaded.',
    "Language set to %1",
    "UI font size set to %1",
    "Open CP/M disk image",
    "Disk images (*.dsk *.ddi *.img *.bin);;All files (*)",
    "Image: %1",
    "%1 (user %2)",
    plural("%n file(s)", "%n files"),
    "Overwrite?",
    "%1 already exists.\nOverwrite it?",
    "Some files could not be copied",
    "Copied %1 file(s) to %2",
    "Put into image",
    "Open a disk image first.",
    "Select one or more files on the right first.",
    "Copied %1 of %2 file(s) into the image",
    "Delete",
    plural("Delete %n selected file(s) from the image?",
           "Delete %n selected files from the image?"),
    "Some files could not be deleted",
    "Add files",
]

MKFSDIALOG = [
    "New CP/M image",
    "Select...",
    "Image file",
    "Format",
    "Boot block (IPL/CCP/BDOS/BIOS)",
    "File %1",
    "Whole capacity size",
    "Skew in boot image",
    "Make",
    "...",
    "Choose boot block file %1",
    "Create CP/M image",
    "Disk images (*.dsk *.ddi *.img *.bin);;All files (*)",
    "Select boot block file %1",
    "All files (*)",
    "Please choose an image file to create.",
    "Please select a format.",
    "Could not create image",
    "Completed!",
]

ABOUTDIALOG = [
    "About CPMToolsQt6",
    "About",
    "License",
    "The licence text could not be loaded from the embedded resources.",
]

CONTEXTS = {
    "MainWindow": MAINWINDOW,
    "MkfsDialog": MKFSDIALOG,
    "AboutDialog": ABOUTDIALOG,
}


# ---------------------------------------------------------------------------
# Translations. Anything omitted falls back to the English source at runtime.
# ---------------------------------------------------------------------------

T = {}

T["ja"] = {
    "CPMToolsQt6": "CPMToolsQt6",
    "Drop a disk image here, or press Select...":
        "ここにディスクイメージをドロップするか、選択... を押してください",
    "Disk image to inspect (.dsk, .ddi, .img)":
        "検査するディスクイメージ (.dsk, .ddi, .img)",
    "Select...": "選択...",
    "Image file": "イメージファイル",
    "Format": "フォーマット",
    "Files inside the image. Drag out to copy them to your desktop, "
    "or drop files here to add them.":
        "イメージ内のファイル。ドラッグしてデスクトップへコピーするか、"
        "ここにファイルをドロップして追加します。",
    "Show": "表示",
    "Files in image": "イメージ内のファイル",
    "Filter": "フィルタ",
    "Files on this computer": "このコンピュータ上のファイル",
    "&Get from image": "イメージから取得(&G)",
    "&Put into image": "イメージへ追加(&P)",
    "&Delete": "削除(&D)",
    "Copy the selected image files to the folder shown on the right":
        "選択したイメージ内のファイルを右側に表示されているフォルダへコピーします",
    "Copy the files selected on the right into the image":
        "右側で選択したファイルをイメージにコピーします",
    "Delete the selected files from the image":
        "選択したファイルをイメージから削除します",
    "&File": "ファイル(&F)",
    "&Open image...": "イメージを開く(&O)...",
    "&New image...": "新しいイメージ(&N)...",
    "E&xit": "終了(&X)",
    "&Edit": "編集(&E)",
    "&Settings": "設定(&S)",
    "UI &Font Size": "UI フォントサイズ(&F)",
    "&Small": "小(&S)",
    "&Medium": "中(&M)",
    "The system default size": "システムの既定サイズ",
    "&Large": "大(&L)",
    "&Extra large": "特大(&E)",
    "&Language": "言語(&L)",
    "Folder path - type or paste, or press Browse...":
        "フォルダパス — 入力または貼り付け、または参照... を押します",
    "Path of the folder shown below. A file path selects that file in its folder.":
        "下に表示するフォルダのパス。ファイルのパスを指定すると、そのファイルを選択します。",
    "Browse...": "参照...",
    "Choose a folder, or a file inside a folder":
        "フォルダ、またはフォルダ内のファイルを選択",
    "Browse this computer": "このコンピュータを参照",
    "What would you like to open?": "何を開きますか？",
    "Folder...": "フォルダ...",
    "File...": "ファイル...",
    "Choose a folder": "フォルダを選択",
    "Choose a file": "ファイルを選択",
    "Folder not found": "フォルダが見つかりません",
    "There is no file or folder at:\n%1":
        "次の場所にファイルまたはフォルダがありません:\n%1",
    "Showing %1": "%1 を表示中",
    "Showing %1 (selected %2)": "%1 を表示中 (%2 を選択)",
    "&System default": "システム既定(&S)",
    "&Help": "ヘルプ(&H)",
    "On&line": "オンライン(&L)",
    "&About": "バージョン情報(&A)",
    "Main": "メイン",
    "Ready": "準備完了",
    "Language": "言語",
    'The translation for "%1" could not be loaded.':
        "「%1」の翻訳を読み込めませんでした。",
    "Language set to %1": "言語を %1 に設定しました",
    "UI font size set to %1": "UI フォントサイズを %1 に設定しました",
    "Open CP/M disk image": "CP/M ディスクイメージを開く",
    "Disk images (*.dsk *.ddi *.img *.bin);;All files (*)":
        "ディスクイメージ (*.dsk *.ddi *.img *.bin);;すべてのファイル (*)",
    "Image: %1": "イメージ: %1",
    "%1 (user %2)": "%1 (ユーザー %2)",
    "%n file(s)": ("%n 個のファイル", "%n 個のファイル"),
    "Overwrite?": "上書きしますか？",
    "%1 already exists.\nOverwrite it?":
        "%1 は既に存在します。\n上書きしますか？",
    "Some files could not be copied": "一部のファイルをコピーできませんでした",
    "Copied %1 file(s) to %2": "%1 個のファイルを %2 にコピーしました",
    "Put into image": "イメージへ追加",
    "Open a disk image first.": "先にディスクイメージを開いてください。",
    "Select one or more files on the right first.":
        "先に右側でファイルを 1 つ以上選択してください。",
    "Copied %1 of %2 file(s) into the image":
        "%2 個中 %1 個のファイルをイメージにコピーしました",
    "Delete": "削除",
    "Delete %n selected file(s) from the image?":
        ("イメージから選択した %n 個のファイルを削除しますか？",
         "イメージから選択した %n 個のファイルを削除しますか？"),
    "Some files could not be deleted": "一部のファイルを削除できませんでした",
    "Add files": "ファイルを追加",
    "New CP/M image": "新しい CP/M イメージ",
    "Boot block (IPL/CCP/BDOS/BIOS)": "ブートブロック (IPL/CCP/BDOS/BIOS)",
    "File %1": "ファイル %1",
    "Whole capacity size": "全容量サイズ",
    "Skew in boot image": "ブートイメージのスキュー",
    "Make": "作成",
    "Choose boot block file %1": "ブートブロックファイル %1 を選択",
    "Create CP/M image": "CP/M イメージを作成",
    "Select boot block file %1": "ブートブロックファイル %1 を選択",
    "All files (*)": "すべてのファイル (*)",
    "Please choose an image file to create.":
        "作成するイメージファイルを選択してください。",
    "Please select a format.": "フォーマットを選択してください。",
    "Could not create image": "イメージを作成できませんでした",
    "Completed!": "完了しました！",
    "About CPMToolsQt6": "CPMToolsQt6 について",
    "About": "バージョン情報",
    "License": "ライセンス",
    "The licence text could not be loaded from the embedded resources.":
        "ライセンス本文を埋め込みリソースから読み込めませんでした。",
}

T["zh_CN"] = {
    "CPMToolsQt6": "CPMToolsQt6",
    "Drop a disk image here, or press Select...":
        "将磁盘映像拖放到此处，或按“选择...”。",
    "Disk image to inspect (.dsk, .ddi, .img)":
        "要查看的磁盘映像 (.dsk, .ddi, .img)",
    "Select...": "选择...",
    "Image file": "映像文件",
    "Format": "格式",
    "Files inside the image. Drag out to copy them to your desktop, "
    "or drop files here to add them.":
        "映像中的文件。拖出可复制到桌面，或拖入文件以添加。",
    "Show": "显示",
    "Files in image": "映像中的文件",
    "Filter": "过滤器",
    "Files on this computer": "此计算机上的文件",
    "&Get from image": "从映像提取(&G)",
    "&Put into image": "写入映像(&P)",
    "&Delete": "删除(&D)",
    "Copy the selected image files to the folder shown on the right":
        "将选定的映像文件复制到右侧显示的文件夹",
    "Copy the files selected on the right into the image":
        "将右侧选定的文件复制到映像中",
    "Delete the selected files from the image": "从映像中删除选定的文件",
    "&File": "文件(&F)",
    "&Open image...": "打开映像(&O)...",
    "&New image...": "新建映像(&N)...",
    "E&xit": "退出(&X)",
    "&Edit": "编辑(&E)",
    "&Settings": "设置(&S)",
    "UI &Font Size": "界面字体大小(&F)",
    "&Small": "小(&S)",
    "&Medium": "中(&M)",
    "The system default size": "系统默认大小",
    "&Large": "大(&L)",
    "&Extra large": "特大(&E)",
    "&Language": "语言(&L)",
    "Folder path - type or paste, or press Browse...":
        "文件夹路径 — 输入或粘贴，或按浏览...",
    "Path of the folder shown below. A file path selects that file in its folder.":
        "下方显示的文件夹路径。输入文件路径将选中该文件。",
    "Browse...": "浏览...",
    "Choose a folder, or a file inside a folder": "选择文件夹，或文件夹中的文件",
    "Browse this computer": "浏览此计算机",
    "What would you like to open?": "您想打开什么？",
    "Folder...": "文件夹...",
    "File...": "文件...",
    "Choose a folder": "选择文件夹",
    "Choose a file": "选择文件",
    "Folder not found": "找不到文件夹",
    "There is no file or folder at:\n%1": "以下位置没有文件或文件夹：\n%1",
    "Showing %1": "正在显示 %1",
    "Showing %1 (selected %2)": "正在显示 %1（已选中 %2）",
    "&System default": "系统默认(&S)",
    "&Help": "帮助(&H)",
    "On&line": "在线(&L)",
    "&About": "关于(&A)",
    "Main": "主工具栏",
    "Ready": "就绪",
    "Language": "语言",
    'The translation for "%1" could not be loaded.':
        "无法加载“%1”的翻译。",
    "Language set to %1": "语言已设置为 %1",
    "UI font size set to %1": "界面字体大小已设置为 %1",
    "Open CP/M disk image": "打开 CP/M 磁盘映像",
    "Disk images (*.dsk *.ddi *.img *.bin);;All files (*)":
        "磁盘映像 (*.dsk *.ddi *.img *.bin);;所有文件 (*)",
    "Image: %1": "映像：%1",
    "%1 (user %2)": "%1（用户 %2）",
    "%n file(s)": ("%n 个文件", "%n 个文件"),
    "Overwrite?": "覆盖？",
    "%1 already exists.\nOverwrite it?":
        "%1 已存在。\n是否覆盖？",
    "Some files could not be copied": "部分文件无法复制",
    "Copied %1 file(s) to %2": "已将 %1 个文件复制到 %2",
    "Put into image": "写入映像",
    "Open a disk image first.": "请先打开一个磁盘映像。",
    "Select one or more files on the right first.":
        "请先在右侧选择一个或多个文件。",
    "Copied %1 of %2 file(s) into the image":
        "已将 %2 个文件中的 %1 个复制到映像",
    "Delete": "删除",
    "Delete %n selected file(s) from the image?":
        ("从映像中删除选定的 %n 个文件？", "从映像中删除选定的 %n 个文件？"),
    "Some files could not be deleted": "部分文件无法删除",
    "Add files": "添加文件",
    "New CP/M image": "新建 CP/M 映像",
    "Boot block (IPL/CCP/BDOS/BIOS)": "引导块 (IPL/CCP/BDOS/BIOS)",
    "File %1": "文件 %1",
    "Whole capacity size": "整个容量大小",
    "Skew in boot image": "引导映像中的偏移",
    "Make": "创建",
    "Choose boot block file %1": "选择引导块文件 %1",
    "Create CP/M image": "创建 CP/M 映像",
    "Select boot block file %1": "选择引导块文件 %1",
    "All files (*)": "所有文件 (*)",
    "Please choose an image file to create.": "请选择要创建的映像文件。",
    "Please select a format.": "请选择一种格式。",
    "Could not create image": "无法创建映像",
    "Completed!": "已完成！",
    "About CPMToolsQt6": "关于 CPMToolsQt6",
    "About": "关于",
    "License": "许可证",
    "The licence text could not be loaded from the embedded resources.":
        "无法从嵌入资源中加载许可证文本。",
}

T["zh_TW"] = {
    "CPMToolsQt6": "CPMToolsQt6",
    "Drop a disk image here, or press Select...":
        "將磁碟映像拖放到此處，或按「選擇...」。",
    "Disk image to inspect (.dsk, .ddi, .img)":
        "要檢視的磁碟映像 (.dsk, .ddi, .img)",
    "Select...": "選擇...",
    "Image file": "映像檔案",
    "Format": "格式",
    "Files inside the image. Drag out to copy them to your desktop, "
    "or drop files here to add them.":
        "映像中的檔案。拖出可複製到桌面，或拖入檔案以新增。",
    "Show": "顯示",
    "Files in image": "映像中的檔案",
    "Filter": "篩選器",
    "Files on this computer": "這部電腦上的檔案",
    "&Get from image": "從映像取出(&G)",
    "&Put into image": "寫入映像(&P)",
    "&Delete": "刪除(&D)",
    "Copy the selected image files to the folder shown on the right":
        "將選取的映像檔案複製到右側顯示的資料夾",
    "Copy the files selected on the right into the image":
        "將右側選取的檔案複製到映像中",
    "Delete the selected files from the image": "從映像中刪除選取的檔案",
    "&File": "檔案(&F)",
    "&Open image...": "開啟映像(&O)...",
    "&New image...": "新增映像(&N)...",
    "E&xit": "結束(&X)",
    "&Edit": "編輯(&E)",
    "&Settings": "設定(&S)",
    "UI &Font Size": "介面字型大小(&F)",
    "&Small": "小(&S)",
    "&Medium": "中(&M)",
    "The system default size": "系統預設大小",
    "&Large": "大(&L)",
    "&Extra large": "特大(&E)",
    "&Language": "語言(&L)",
    "Folder path - type or paste, or press Browse...":
        "資料夾路徑 — 輸入或貼上，或按瀏覽...",
    "Path of the folder shown below. A file path selects that file in its folder.":
        "下方顯示的資料夾路徑。輸入檔案路徑將選取該檔案。",
    "Browse...": "瀏覽...",
    "Choose a folder, or a file inside a folder": "選擇資料夾，或資料夾中的檔案",
    "Browse this computer": "瀏覽這部電腦",
    "What would you like to open?": "您想開啟什麼？",
    "Folder...": "資料夾...",
    "File...": "檔案...",
    "Choose a folder": "選擇資料夾",
    "Choose a file": "選擇檔案",
    "Folder not found": "找不到資料夾",
    "There is no file or folder at:\n%1": "以下位置沒有檔案或資料夾：\n%1",
    "Showing %1": "正在顯示 %1",
    "Showing %1 (selected %2)": "正在顯示 %1（已選取 %2）",
    "&System default": "系統預設(&S)",
    "&Help": "說明(&H)",
    "On&line": "線上(&L)",
    "&About": "關於(&A)",
    "Main": "主工具列",
    "Ready": "就緒",
    "Language": "語言",
    'The translation for "%1" could not be loaded.':
        "無法載入「%1」的翻譯。",
    "Language set to %1": "語言已設定為 %1",
    "UI font size set to %1": "介面字型大小已設定為 %1",
    "Open CP/M disk image": "開啟 CP/M 磁碟映像",
    "Disk images (*.dsk *.ddi *.img *.bin);;All files (*)":
        "磁碟映像 (*.dsk *.ddi *.img *.bin);;所有檔案 (*)",
    "Image: %1": "映像：%1",
    "%1 (user %2)": "%1（使用者 %2）",
    "%n file(s)": ("%n 個檔案", "%n 個檔案"),
    "Overwrite?": "要覆寫嗎？",
    "%1 already exists.\nOverwrite it?":
        "%1 已存在。\n要覆寫嗎？",
    "Some files could not be copied": "部分檔案無法複製",
    "Copied %1 file(s) to %2": "已將 %1 個檔案複製到 %2",
    "Put into image": "寫入映像",
    "Open a disk image first.": "請先開啟磁碟映像。",
    "Select one or more files on the right first.":
        "請先在右側選取一或多個檔案。",
    "Copied %1 of %2 file(s) into the image":
        "已將 %2 個檔案中的 %1 個複製到映像",
    "Delete": "刪除",
    "Delete %n selected file(s) from the image?":
        ("從映像中刪除選取的 %n 個檔案？", "從映像中刪除選取的 %n 個檔案？"),
    "Some files could not be deleted": "部分檔案無法刪除",
    "Add files": "新增檔案",
    "New CP/M image": "新增 CP/M 映像",
    "Boot block (IPL/CCP/BDOS/BIOS)": "開機磁區 (IPL/CCP/BDOS/BIOS)",
    "File %1": "檔案 %1",
    "Whole capacity size": "完整容量大小",
    "Skew in boot image": "開機映像的偏移",
    "Make": "建立",
    "Choose boot block file %1": "選擇開機磁區檔案 %1",
    "Create CP/M image": "建立 CP/M 映像",
    "Select boot block file %1": "選擇開機磁區檔案 %1",
    "All files (*)": "所有檔案 (*)",
    "Please choose an image file to create.": "請選擇要建立的映像檔案。",
    "Please select a format.": "請選擇一種格式。",
    "Could not create image": "無法建立映像",
    "Completed!": "已完成！",
    "About CPMToolsQt6": "關於 CPMToolsQt6",
    "About": "關於",
    "License": "授權條款",
    "The licence text could not be loaded from the embedded resources.":
        "無法從內嵌資源載入授權條款文字。",
}

T["it"] = {
    "CPMToolsQt6": "CPMToolsQt6",
    "Drop a disk image here, or press Select...":
        "Trascina qui un'immagine disco o premi Seleziona...",
    "Disk image to inspect (.dsk, .ddi, .img)":
        "Immagine disco da esaminare (.dsk, .ddi, .img)",
    "Select...": "Seleziona...",
    "Image file": "File immagine",
    "Format": "Formato",
    "Files inside the image. Drag out to copy them to your desktop, "
    "or drop files here to add them.":
        "File contenuti nell'immagine. Trascinali fuori per copiarli sul "
        "desktop, oppure rilascia qui dei file per aggiungerli.",
    "Show": "Mostra",
    "Files in image": "File nell'immagine",
    "Filter": "Filtro",
    "Files on this computer": "File su questo computer",
    "&Get from image": "&Estrai dall'immagine",
    "&Put into image": "&Inserisci nell'immagine",
    "&Delete": "&Elimina",
    "Copy the selected image files to the folder shown on the right":
        "Copia i file selezionati dall'immagine alla cartella a destra",
    "Copy the files selected on the right into the image":
        "Copia nell'immagine i file selezionati a destra",
    "Folder path - type or paste, or press Browse...":
        "Percorso cartella: digitare o incollare, oppure premere Sfoglia...",
    "Path of the folder shown below. A file path selects that file in its folder.":
        "Percorso della cartella mostrata sotto. Un percorso di file seleziona quel file nella sua cartella.",
    "Browse...": "Sfoglia...",
    "Choose a folder, or a file inside a folder": "Scegli una cartella o un file al suo interno",
    "Browse this computer": "Sfoglia questo computer",
    "What would you like to open?": "Cosa vuoi aprire?",
    "Folder...": "Cartella...",
    "File...": "File...",
    "Choose a folder": "Scegli una cartella",
    "Choose a file": "Scegli un file",
    "Folder not found": "Cartella non trovata",
    "There is no file or folder at:\n%1": "Non esiste alcun file o cartella in:\n%1",
    "Showing %1": "Visualizzazione di %1",
    "Showing %1 (selected %2)": "Visualizzazione di %1 (%2 selezionato)",
    "Delete the selected files from the image":
        "Elimina dall'immagine i file selezionati",
    "Delete the selected files from the image":
        "Elimina dall'immagine i file selezionati",
    "&File": "&File",
    "&Open image...": "&Apri immagine...",
    "&New image...": "&Nuova immagine...",
    "E&xit": "&Esci",
    "&Edit": "&Modifica",
    "&Settings": "&Impostazioni",
    "UI &Font Size": "&Dimensione carattere",
    "&Small": "&Piccola",
    "&Medium": "&Media",
    "The system default size": "La dimensione predefinita di sistema",
    "&Large": "&Grande",
    "&Extra large": "&Molto grande",
    "&Language": "&Lingua",
    "&System default": "&Predefinita di sistema",
    "&Help": "&?",
    "On&line": "&Online",
    "&About": "&Informazioni",
    "Main": "Principale",
    "Ready": "Pronto",
    "Language": "Lingua",
    'The translation for "%1" could not be loaded.':
        'Impossibile caricare la traduzione per "%1".',
    "Language set to %1": "Lingua impostata su %1",
    "UI font size set to %1": "Dimensione carattere impostata su %1",
    "Open CP/M disk image": "Apri immagine disco CP/M",
    "Disk images (*.dsk *.ddi *.img *.bin);;All files (*)":
        "Immagini disco (*.dsk *.ddi *.img *.bin);;Tutti i file (*)",
    "Image: %1": "Immagine: %1",
    "%1 (user %2)": "%1 (utente %2)",
    "%n file(s)": ("%n file", "%n file"),
    "Overwrite?": "Sovrascrivere?",
    "%1 already exists.\nOverwrite it?":
        "%1 esiste già.\nSovrascriverlo?",
    "Some files could not be copied": "Alcuni file non sono stati copiati",
    "Copied %1 file(s) to %2": "%1 file copiati in %2",
    "Put into image": "Inserisci nell'immagine",
    "Open a disk image first.": "Apri prima un'immagine disco.",
    "Select one or more files on the right first.":
        "Seleziona prima uno o più file sulla destra.",
    "Copied %1 of %2 file(s) into the image":
        "%1 file su %2 copiati nell'immagine",
    "Delete": "Elimina",
    "Delete %n selected file(s) from the image?":
        ("Eliminare %n file selezionato dall'immagine?",
         "Eliminare %n file selezionati dall'immagine?"),
    "Some files could not be deleted": "Alcuni file non sono stati eliminati",
    "Add files": "Aggiungi file",
    "New CP/M image": "Nuova immagine CP/M",
    "Boot block (IPL/CCP/BDOS/BIOS)": "Blocco di avvio (IPL/CCP/BDOS/BIOS)",
    "File %1": "File %1",
    "Whole capacity size": "Dimensione capacità totale",
    "Skew in boot image": "Skew nell'immagine di avvio",
    "Make": "Crea",
    "Choose boot block file %1": "Scegli il file blocco di avvio %1",
    "Create CP/M image": "Crea immagine CP/M",
    "Select boot block file %1": "Seleziona il file blocco di avvio %1",
    "All files (*)": "Tutti i file (*)",
    "Please choose an image file to create.":
        "Scegli un file immagine da creare.",
    "Please select a format.": "Seleziona un formato.",
    "Could not create image": "Impossibile creare l'immagine",
    "Completed!": "Completato!",
    "About CPMToolsQt6": "Informazioni su CPMToolsQt6",
    "About": "Informazioni",
    "License": "Licenza",
    "The licence text could not be loaded from the embedded resources.":
        "Impossibile caricare il testo della licenza dalle risorse integrate.",
}

T["fr"] = {
    "CPMToolsQt6": "CPMToolsQt6",
    "Drop a disk image here, or press Select...":
        "Déposez une image disque ici ou appuyez sur Sélectionner...",
    "Disk image to inspect (.dsk, .ddi, .img)":
        "Image disque à examiner (.dsk, .ddi, .img)",
    "Select...": "Sélectionner...",
    "Image file": "Fichier image",
    "Format": "Format",
    "Files inside the image. Drag out to copy them to your desktop, "
    "or drop files here to add them.":
        "Fichiers contenus dans l'image. Faites-les glisser pour les copier "
        "sur le bureau, ou déposez des fichiers ici pour les ajouter.",
    "Show": "Afficher",
    "Files in image": "Fichiers dans l'image",
    "Filter": "Filtre",
    "Files on this computer": "Fichiers sur cet ordinateur",
    "&Get from image": "E&xtraire de l'image",
    "&Put into image": "&Insérer dans l'image",
    "&Delete": "&Supprimer",
    "Copy the selected image files to the folder shown on the right":
        "Copier les fichiers sélectionnés vers le dossier affiché à droite",
    "Copy the files selected on the right into the image":
        "Copier dans l'image les fichiers sélectionnés à droite",
    "Folder path - type or paste, or press Browse...":
        "Chemin du dossier : saisissez ou collez, ou appuyez sur Parcourir...",
    "Path of the folder shown below. A file path selects that file in its folder.":
        "Chemin du dossier affiché ci-dessous. Un chemin de fichier sélectionne ce fichier dans son dossier.",
    "Browse...": "Parcourir...",
    "Choose a folder, or a file inside a folder": "Choisissez un dossier ou un fichier qu'il contient",
    "Browse this computer": "Parcourir cet ordinateur",
    "What would you like to open?": "Que souhaitez-vous ouvrir ?",
    "Folder...": "Dossier...",
    "File...": "Fichier...",
    "Choose a folder": "Choisir un dossier",
    "Choose a file": "Choisir un fichier",
    "Folder not found": "Dossier introuvable",
    "There is no file or folder at:\n%1": "Aucun fichier ou dossier à :\n%1",
    "Showing %1": "Affichage de %1",
    "Showing %1 (selected %2)": "Affichage de %1 (%2 sélectionné)",
    "Delete the selected files from the image":
        "Supprimer de l'image les fichiers sélectionnés",
    "&File": "&Fichier",
    "&Open image...": "&Ouvrir une image...",
    "&New image...": "&Nouvelle image...",
    "E&xit": "&Quitter",
    "&Edit": "&Édition",
    "&Settings": "&Paramètres",
    "UI &Font Size": "&Taille de police",
    "&Small": "&Petite",
    "&Medium": "&Moyenne",
    "The system default size": "La taille par défaut du système",
    "&Large": "&Grande",
    "&Extra large": "&Très grande",
    "&Language": "&Langue",
    "&System default": "&Par défaut du système",
    "&Help": "&Aide",
    "On&line": "En &ligne",
    "&About": "À &propos",
    "Main": "Principal",
    "Ready": "Prêt",
    "Language": "Langue",
    'The translation for "%1" could not be loaded.':
        'Impossible de charger la traduction pour « %1 ».',
    "Language set to %1": "Langue définie sur %1",
    "UI font size set to %1": "Taille de police définie sur %1",
    "Open CP/M disk image": "Ouvrir une image disque CP/M",
    "Disk images (*.dsk *.ddi *.img *.bin);;All files (*)":
        "Images disque (*.dsk *.ddi *.img *.bin);;Tous les fichiers (*)",
    "Image: %1": "Image : %1",
    "%1 (user %2)": "%1 (utilisateur %2)",
    "%n file(s)": ("%n fichier", "%n fichiers"),
    "Overwrite?": "Remplacer ?",
    "%1 already exists.\nOverwrite it?":
        "%1 existe déjà.\nLe remplacer ?",
    "Some files could not be copied": "Certains fichiers n'ont pas été copiés",
    "Copied %1 file(s) to %2": "%1 fichier(s) copié(s) vers %2",
    "Put into image": "Insérer dans l'image",
    "Open a disk image first.": "Ouvrez d'abord une image disque.",
    "Select one or more files on the right first.":
        "Sélectionnez d'abord un ou plusieurs fichiers à droite.",
    "Copied %1 of %2 file(s) into the image":
        "%1 fichier(s) sur %2 copié(s) dans l'image",
    "Delete": "Supprimer",
    "Delete %n selected file(s) from the image?":
        ("Supprimer %n fichier sélectionné de l'image ?",
         "Supprimer %n fichiers sélectionnés de l'image ?"),
    "Some files could not be deleted":
        "Certains fichiers n'ont pas été supprimés",
    "Add files": "Ajouter des fichiers",
    "New CP/M image": "Nouvelle image CP/M",
    "Boot block (IPL/CCP/BDOS/BIOS)": "Bloc d'amorçage (IPL/CCP/BDOS/BIOS)",
    "File %1": "Fichier %1",
    "Whole capacity size": "Taille de capacité totale",
    "Skew in boot image": "Décalage dans l'image d'amorçage",
    "Make": "Créer",
    "Choose boot block file %1": "Choisir le fichier de bloc d'amorçage %1",
    "Create CP/M image": "Créer une image CP/M",
    "Select boot block file %1": "Sélectionner le fichier de bloc d'amorçage %1",
    "All files (*)": "Tous les fichiers (*)",
    "Please choose an image file to create.":
        "Veuillez choisir un fichier image à créer.",
    "Please select a format.": "Veuillez sélectionner un format.",
    "Could not create image": "Impossible de créer l'image",
    "Completed!": "Terminé !",
    "About CPMToolsQt6": "À propos de CPMToolsQt6",
    "About": "À propos",
    "License": "Licence",
    "The licence text could not be loaded from the embedded resources.":
        "Impossible de charger le texte de la licence depuis les ressources "
        "intégrées.",
}

T["de"] = {
    "CPMToolsQt6": "CPMToolsQt6",
    "Drop a disk image here, or press Select...":
        "Disk-Image hierher ziehen oder Auswählen... drücken",
    "Disk image to inspect (.dsk, .ddi, .img)":
        "Zu untersuchendes Disk-Image (.dsk, .ddi, .img)",
    "Select...": "Auswählen...",
    "Image file": "Imagedatei",
    "Format": "Format",
    "Files inside the image. Drag out to copy them to your desktop, "
    "or drop files here to add them.":
        "Dateien im Image. Zum Kopieren auf den Desktop herausziehen oder "
        "Dateien zum Hinzufügen hier ablegen.",
    "Show": "Anzeigen",
    "Files in image": "Dateien im Image",
    "Filter": "Filter",
    "Files on this computer": "Dateien auf diesem Computer",
    "&Get from image": "Aus Image &extrahieren",
    "&Put into image": "In Image &einfügen",
    "&Delete": "&Löschen",
    "Copy the selected image files to the folder shown on the right":
        "Die ausgewählten Dateien in den rechts angezeigten Ordner kopieren",
    "Copy the files selected on the right into the image":
        "Die rechts ausgewählten Dateien in das Image kopieren",
    "Folder path - type or paste, or press Browse...":
        "Ordnerpfad – eingeben oder einfügen, oder Durchsuchen... drücken",
    "Path of the folder shown below. A file path selects that file in its folder.":
        "Pfad des unten angezeigten Ordners. Ein Dateipfad wählt diese Datei in ihrem Ordner aus.",
    "Browse...": "Durchsuchen...",
    "Choose a folder, or a file inside a folder": "Ordner oder eine darin liegende Datei wählen",
    "Browse this computer": "Diesen Computer durchsuchen",
    "What would you like to open?": "Was möchten Sie öffnen?",
    "Folder...": "Ordner...",
    "File...": "Datei...",
    "Choose a folder": "Ordner wählen",
    "Choose a file": "Datei wählen",
    "Folder not found": "Ordner nicht gefunden",
    "There is no file or folder at:\n%1": "Es gibt keine Datei und keinen Ordner unter:\n%1",
    "Showing %1": "%1 wird angezeigt",
    "Showing %1 (selected %2)": "%1 wird angezeigt (%2 ausgewählt)",
    "Delete the selected files from the image":
        "Die ausgewählten Dateien aus dem Image löschen",
    "&File": "&Datei",
    "&Open image...": "Image &öffnen...",
    "&New image...": "&Neues Image...",
    "E&xit": "&Beenden",
    "&Edit": "&Bearbeiten",
    "&Settings": "&Einstellungen",
    "UI &Font Size": "&Schriftgröße",
    "&Small": "&Klein",
    "&Medium": "&Mittel",
    "The system default size": "Die Standardgröße des Systems",
    "&Large": "&Groß",
    "&Extra large": "&Sehr groß",
    "&Language": "&Sprache",
    "&System default": "&Systemstandard",
    "&Help": "&Hilfe",
    "On&line": "&Online",
    "&About": "&Über",
    "Main": "Haupt",
    "Ready": "Bereit",
    "Language": "Sprache",
    'The translation for "%1" could not be loaded.':
        'Die Übersetzung für „%1" konnte nicht geladen werden.',
    "Language set to %1": "Sprache auf %1 gesetzt",
    "UI font size set to %1": "Schriftgröße auf %1 gesetzt",
    "Open CP/M disk image": "CP/M-Disk-Image öffnen",
    "Disk images (*.dsk *.ddi *.img *.bin);;All files (*)":
        "Disk-Images (*.dsk *.ddi *.img *.bin);;Alle Dateien (*)",
    "Image: %1": "Image: %1",
    "%1 (user %2)": "%1 (Benutzer %2)",
    "%n file(s)": ("%n Datei", "%n Dateien"),
    "Overwrite?": "Überschreiben?",
    "%1 already exists.\nOverwrite it?":
        "%1 ist bereits vorhanden.\nÜberschreiben?",
    "Some files could not be copied":
        "Einige Dateien konnten nicht kopiert werden",
    "Copied %1 file(s) to %2": "%1 Datei(en) nach %2 kopiert",
    "Put into image": "In Image einfügen",
    "Open a disk image first.": "Bitte zuerst ein Disk-Image öffnen.",
    "Select one or more files on the right first.":
        "Bitte zuerst rechts eine oder mehrere Dateien auswählen.",
    "Copied %1 of %2 file(s) into the image":
        "%1 von %2 Datei(en) in das Image kopiert",
    "Delete": "Löschen",
    "Delete %n selected file(s) from the image?":
        ("%n ausgewählte Datei aus dem Image löschen?",
         "%n ausgewählte Dateien aus dem Image löschen?"),
    "Some files could not be deleted":
        "Einige Dateien konnten nicht gelöscht werden",
    "Add files": "Dateien hinzufügen",
    "New CP/M image": "Neues CP/M-Image",
    "Boot block (IPL/CCP/BDOS/BIOS)": "Bootblock (IPL/CCP/BDOS/BIOS)",
    "File %1": "Datei %1",
    "Whole capacity size": "Gesamte Kapazitätsgröße",
    "Skew in boot image": "Skew im Boot-Image",
    "Make": "Erstellen",
    "Choose boot block file %1": "Bootblock-Datei %1 wählen",
    "Create CP/M image": "CP/M-Image erstellen",
    "Select boot block file %1": "Bootblock-Datei %1 auswählen",
    "All files (*)": "Alle Dateien (*)",
    "Please choose an image file to create.":
        "Bitte eine zu erstellende Imagedatei wählen.",
    "Please select a format.": "Bitte ein Format wählen.",
    "Could not create image": "Image konnte nicht erstellt werden",
    "Completed!": "Abgeschlossen!",
    "About CPMToolsQt6": "Über CPMToolsQt6",
    "About": "Über",
    "License": "Lizenz",
    "The licence text could not be loaded from the embedded resources.":
        "Der Lizenztext konnte nicht aus den eingebetteten Ressourcen "
        "geladen werden.",
}

T["es"] = {
    "CPMToolsQt6": "CPMToolsQt6",
    "Drop a disk image here, or press Select...":
        "Arrastra aquí una imagen de disco o pulsa Seleccionar...",
    "Disk image to inspect (.dsk, .ddi, .img)":
        "Imagen de disco a examinar (.dsk, .ddi, .img)",
    "Select...": "Seleccionar...",
    "Image file": "Archivo de imagen",
    "Format": "Formato",
    "Files inside the image. Drag out to copy them to your desktop, "
    "or drop files here to add them.":
        "Archivos dentro de la imagen. Arrástralos fuera para copiarlos al "
        "escritorio o suelta archivos aquí para añadirlos.",
    "Show": "Mostrar",
    "Files in image": "Archivos en la imagen",
    "Filter": "Filtro",
    "Files on this computer": "Archivos en este equipo",
    "&Get from image": "E&xtraer de la imagen",
    "&Put into image": "&Insertar en la imagen",
    "&Delete": "&Eliminar",
    "Copy the selected image files to the folder shown on the right":
        "Copiar los archivos seleccionados a la carpeta mostrada a la derecha",
    "Copy the files selected on the right into the image":
        "Copiar en la imagen los archivos seleccionados a la derecha",
    "Folder path - type or paste, or press Browse...":
        "Ruta de carpeta: escriba o pegue, o pulse Examinar...",
    "Path of the folder shown below. A file path selects that file in its folder.":
        "Ruta de la carpeta mostrada abajo. Una ruta de archivo selecciona ese archivo en su carpeta.",
    "Browse...": "Examinar...",
    "Choose a folder, or a file inside a folder": "Elija una carpeta o un archivo dentro de ella",
    "Browse this computer": "Examinar este equipo",
    "What would you like to open?": "¿Qué desea abrir?",
    "Folder...": "Carpeta...",
    "File...": "Archivo...",
    "Choose a folder": "Elegir una carpeta",
    "Choose a file": "Elegir un archivo",
    "Folder not found": "Carpeta no encontrada",
    "There is no file or folder at:\n%1": "No hay ningún archivo ni carpeta en:\n%1",
    "Showing %1": "Mostrando %1",
    "Showing %1 (selected %2)": "Mostrando %1 (%2 seleccionado)",
    "Delete the selected files from the image":
        "Eliminar de la imagen los archivos seleccionados",
    "&File": "&Archivo",
    "&Open image...": "&Abrir imagen...",
    "&New image...": "&Nueva imagen...",
    "E&xit": "&Salir",
    "&Edit": "&Edición",
    "&Settings": "&Configuración",
    "UI &Font Size": "&Tamaño de fuente",
    "&Small": "&Pequeña",
    "&Medium": "&Mediana",
    "The system default size": "El tamaño predeterminado del sistema",
    "&Large": "&Grande",
    "&Extra large": "Muy &grande",
    "&Language": "&Idioma",
    "&System default": "Pre&determinado del sistema",
    "&Help": "A&yuda",
    "On&line": "En &línea",
    "&About": "&Acerca de",
    "Main": "Principal",
    "Ready": "Listo",
    "Language": "Idioma",
    'The translation for "%1" could not be loaded.':
        'No se pudo cargar la traducción de «%1».',
    "Language set to %1": "Idioma establecido en %1",
    "UI font size set to %1": "Tamaño de fuente establecido en %1",
    "Open CP/M disk image": "Abrir imagen de disco CP/M",
    "Disk images (*.dsk *.ddi *.img *.bin);;All files (*)":
        "Imágenes de disco (*.dsk *.ddi *.img *.bin);;Todos los archivos (*)",
    "Image: %1": "Imagen: %1",
    "%1 (user %2)": "%1 (usuario %2)",
    "%n file(s)": ("%n archivo", "%n archivos"),
    "Overwrite?": "¿Sobrescribir?",
    "%1 already exists.\nOverwrite it?":
        "%1 ya existe.\n¿Sobrescribirlo?",
    "Some files could not be copied": "No se pudieron copiar algunos archivos",
    "Copied %1 file(s) to %2": "%1 archivo(s) copiado(s) a %2",
    "Put into image": "Insertar en la imagen",
    "Open a disk image first.": "Abre primero una imagen de disco.",
    "Select one or more files on the right first.":
        "Selecciona primero uno o más archivos a la derecha.",
    "Copied %1 of %2 file(s) into the image":
        "%1 de %2 archivo(s) copiado(s) en la imagen",
    "Delete": "Eliminar",
    "Delete %n selected file(s) from the image?":
        ("¿Eliminar %n archivo seleccionado de la imagen?",
         "¿Eliminar %n archivos seleccionados de la imagen?"),
    "Some files could not be deleted":
        "No se pudieron eliminar algunos archivos",
    "Add files": "Añadir archivos",
    "New CP/M image": "Nueva imagen CP/M",
    "Boot block (IPL/CCP/BDOS/BIOS)": "Bloque de arranque (IPL/CCP/BDOS/BIOS)",
    "File %1": "Archivo %1",
    "Whole capacity size": "Tamaño de capacidad total",
    "Skew in boot image": "Desplazamiento en la imagen de arranque",
    "Make": "Crear",
    "Choose boot block file %1": "Elegir el archivo de bloque de arranque %1",
    "Create CP/M image": "Crear imagen CP/M",
    "Select boot block file %1":
        "Seleccionar el archivo de bloque de arranque %1",
    "All files (*)": "Todos los archivos (*)",
    "Please choose an image file to create.":
        "Elige un archivo de imagen para crear.",
    "Please select a format.": "Selecciona un formato.",
    "Could not create image": "No se pudo crear la imagen",
    "Completed!": "¡Completado!",
    "About CPMToolsQt6": "Acerca de CPMToolsQt6",
    "About": "Acerca de",
    "License": "Licencia",
    "The licence text could not be loaded from the embedded resources.":
        "No se pudo cargar el texto de la licencia desde los recursos "
        "integrados.",
}


# English is the source language: its catalogue is empty on purpose, so that
# selecting English simply shows the original strings.
T["en"] = {}


def prettify(elem):
    """Return a pretty-printed XML string."""
    rough = ET.tostring(elem, encoding="utf-8")
    reparsed = minidom.parseString(rough)
    return reparsed.toprettyxml(indent="    ", encoding="utf-8").decode("utf-8")


def build_ts(lang_code, qt_lang):
    ts = ET.Element("TS", {
        "version": "2.1",
        "language": qt_lang,
        "sourcelanguage": "en_US",
    })
    table = T.get(lang_code, {})

    for context_name, entries in CONTEXTS.items():
        context = ET.SubElement(ts, "context")
        ET.SubElement(context, "name").text = context_name

        for entry in entries:
            is_plural = isinstance(entry, tuple) and entry[0] is PLURAL
            source = entry[1] if is_plural else entry

            message = ET.SubElement(context, "message")
            if is_plural:
                message.set("numerus", "yes")
            ET.SubElement(message, "source").text = source

            translation = table.get(source)

            node = ET.SubElement(message, "translation")
            if is_plural:
                if translation is None:
                    node.set("type", "unfinished")
                else:
                    singular, plural_form = translation
                    ET.SubElement(node, "numerusform").text = singular
                    if lang_code not in SINGLE_PLURAL:
                        ET.SubElement(node, "numerusform").text = plural_form
            else:
                if translation is None:
                    node.set("type", "unfinished")
                else:
                    node.text = translation

    return prettify(ts)


def main():
    os.makedirs(OUT_DIR, exist_ok=True)

    total = 0
    for lang_code, qt_lang in LANGUAGES:
        xml = build_ts(lang_code, qt_lang)
        path = os.path.join(OUT_DIR, f"cpmtoolsgui_{lang_code}.ts")
        with open(path, "w", encoding="utf-8", newline="\n") as handle:
            handle.write(xml)

        count = len(T.get(lang_code, {}))
        total += count
        print(f"  {os.path.basename(path):32} {count:4} translated")

    print(f"\nWrote {len(LANGUAGES)} catalogues, {total} translations.")


if __name__ == "__main__":
    main()
