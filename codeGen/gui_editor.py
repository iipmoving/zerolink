#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
gui_editor.py — v2.3 LINK+PARAMS 配置编辑器 (Tkinter)

功能:
  - 打开/保存 project.json
  - 模块树浏览 + 管道选择
  - 管道属性编辑 (callback_type, LINK style, array_size)
  - 字段 Table 编辑 (名称/类型/注释)
  - 代码预览
  - 一键生成 (全部 / 仅 io.h / 仅 switcher / 仅 modules)
"""

import tkinter as tk
from tkinter import ttk, filedialog, messagebox
import json
import os
import sys
import json
import subprocess
import shutil
import difflib

# 生成器引擎 (直接导入，非子进程)
from gen_io_h import generate_io_h, _pascal_to_snake
from gen_switcher import generate_switcher
from gen_module_c import generate_module_c, generate_module_h, generate_module_c_refactored, strip_ai_blocks, wrap_in_ai_block, replace_ai_block, find_ai_block

# KEIL 项目解析器
from keil_parser import parse_uvprojx, filter_files, scan_source_files, generate_ai_scan_doc


class ConfigEditor:
    """主编辑器窗口"""

    def __init__(self, root):
        self.root = root
        self.root.title("v2.3 LINK+PARAMS 配置编辑器")
        self.root.geometry("1200x780")

        # ---- 数据 ----
        self.config_path = None
        self.config = None
        self.modules = []
        self.pipes = []
        self.slot_order = []
        self._selected_pipe = None  # 当前选中的 pipe dict (引用)
        self._field_item_map = {}   # {Treeview iid: field_dict} 用于嵌套字段编辑

        # ---- 构建 UI ----
        self._build_menu()
        self._build_main_area()
        self._build_bottom_bar()

        self.status("就绪 — 打开 JSON 配置文件开始编辑")

    # ================================================================
    # 菜单
    # ================================================================
    def _build_menu(self):
        menubar = tk.Menu(self.root)
        self.root.config(menu=menubar)

        fm = tk.Menu(menubar, tearoff=0)
        fm.add_command(label="打开 JSON...", command=self.cmd_open, accelerator="Ctrl+O")
        fm.add_command(label="保存 JSON", command=self.cmd_save, accelerator="Ctrl+S")
        fm.add_command(label="另存为...", command=self.cmd_save_as)
        fm.add_separator()
        fm.add_command(label="退出", command=self.root.quit)
        menubar.add_cascade(label="文件", menu=fm)

        tm = tk.Menu(menubar, tearoff=0)
        tm.add_command(label="合规检查", command=self.cmd_check, accelerator="Ctrl+R")
        tm.add_separator()
        tm.add_command(label="全部生成", command=lambda: self.cmd_generate("all"))
        tm.add_command(label="仅 io.h", command=lambda: self.cmd_generate("io"))
        tm.add_command(label="仅 switcher", command=lambda: self.cmd_generate("switcher"))
        tm.add_command(label="仅 modules", command=lambda: self.cmd_generate("modules"))
        menubar.add_cascade(label="工具", menu=tm)

        hm = tk.Menu(menubar, tearoff=0)
        hm.add_command(label="关于", command=self.cmd_about)
        menubar.add_cascade(label="帮助", menu=hm)

        # 快捷键
        self.root.bind("<Control-o>", lambda e: self.cmd_open())
        self.root.bind("<Control-s>", lambda e: self.cmd_save())
        self.root.bind("<Control-r>", lambda e: self.cmd_check())

    # ================================================================
    # 主布局
    # ================================================================
    def _build_main_area(self):
        """左右分栏: 左=项目设置+模块树, 右=管道编辑+代码预览"""
        main_pw = ttk.PanedWindow(self.root, orient=tk.HORIZONTAL)
        main_pw.pack(fill=tk.BOTH, expand=True, padx=4, pady=4)

        # ---- 左侧面板 ----
        left_frame = ttk.Frame(main_pw)
        main_pw.add(left_frame, weight=1)

        self._build_project_frame(left_frame)
        self._build_tree_frame(left_frame)

        # ---- 右侧面板 ----
        right_frame = ttk.Frame(main_pw)
        main_pw.add(right_frame, weight=2)

        self._build_editor_notebook(right_frame)

    # ----- 项目设置 -----
    def _build_project_frame(self, parent):
        pf = ttk.LabelFrame(parent, text="项目设置", padding=6)
        pf.pack(fill=tk.X, pady=(0, 4))

        row = 0
        ttk.Label(pf, text="输出根目录:").grid(row=row, column=0, sticky=tk.W)
        self.entry_output_root = ttk.Entry(pf)
        self.entry_output_root.grid(row=row, column=1, sticky=tk.EW, padx=4)
        pf.columnconfigure(1, weight=1)
        row += 1

        for label, key in [("io_dir:", "io_dir"), ("core_dir:", "core_dir"),
                           ("app_dir:", "app_dir"), ("base_dir:", "base_class_dir"),
                           ("proto_dir:", "proto_dir")]:
            ttk.Label(pf, text=label).grid(row=row, column=0, sticky=tk.W)
            ent = ttk.Entry(pf)
            ent.grid(row=row, column=1, sticky=tk.EW, padx=4)
            setattr(self, f"entry_{key.replace('.', '_')}", ent)
            row += 1

        ttk.Label(pf, text="pack:").grid(row=row, column=0, sticky=tk.W)
        self.entry_pack = ttk.Entry(pf, width=6)
        self.entry_pack.grid(row=row, column=1, sticky=tk.W, padx=4)

    # ----- 模块树 -----
    def _build_tree_frame(self, parent):
        tf = ttk.LabelFrame(parent, text="模块列表", padding=4)
        tf.pack(fill=tk.BOTH, expand=True)

        toolbar = ttk.Frame(tf)
        toolbar.pack(fill=tk.X, pady=(0, 4))
        ttk.Button(toolbar, text="+ 模块", command=self.cmd_add_module, width=8).pack(side=tk.LEFT, padx=2)
        ttk.Button(toolbar, text="+ 管道", command=self.cmd_add_pipe, width=8).pack(side=tk.LEFT, padx=2)
        ttk.Button(toolbar, text="- 模块", command=self.cmd_delete_module, width=8).pack(side=tk.LEFT, padx=2)
        ttk.Button(toolbar, text="- 管道", command=self.cmd_delete_pipe, width=8).pack(side=tk.LEFT, padx=2)

        self.tree = ttk.Treeview(tf, columns=("type",), displaycolumns=(), height=14)
        self.tree.pack(fill=tk.BOTH, expand=True)
        self.tree.bind("<<TreeviewSelect>>", self._on_tree_select)

        vsb = ttk.Scrollbar(tf, orient=tk.VERTICAL, command=self.tree.yview)
        vsb.pack(side=tk.RIGHT, fill=tk.Y)
        self.tree.configure(yscrollcommand=vsb.set)

    # ----- 右侧 Notebook (管道编辑 + 代码预览 + 检查结果) -----
    def _build_editor_notebook(self, parent):
        self.notebook = ttk.Notebook(parent)
        self.notebook.pack(fill=tk.BOTH, expand=True)

        # Tab 1: 管道 + 字段编辑
        self._build_pipe_tab()

        # Tab 2: 管道预览 (与管道选择关联)
        self._build_pipe_preview_tab()

        # Tab 3: 代码预览 (与模块选择关联)
        self._build_code_preview_tab()

        # Tab 4: 检查结果
        self._build_check_tab()

        # Tab 5: 重构工作流
        self._build_refactor_tab()

    def _build_pipe_tab(self):
        tab = ttk.Frame(self.notebook)
        self.notebook.add(tab, text="管道 / 字段")

        # -- 管道属性 --
        pf = ttk.LabelFrame(tab, text="管道属性", padding=6)
        pf.pack(fill=tk.X, pady=(0, 6))

        ttk.Label(pf, text="管道:").grid(row=0, column=0, sticky=tk.W)
        self.lbl_pipe_name = ttk.Label(pf, text="(未选择)")
        self.lbl_pipe_name.grid(row=0, column=1, sticky=tk.W, padx=4)

        ttk.Label(pf, text="callback_type:").grid(row=1, column=0, sticky=tk.W)
        self.cb_callback_type = ttk.Combobox(pf, values=["pull", "edge", "field_copy"], state="readonly", width=14)
        self.cb_callback_type.grid(row=1, column=1, sticky=tk.W, padx=4)
        self.cb_callback_type.bind("<<ComboboxSelected>>", self._on_pipe_prop_change)

        ttk.Label(pf, text="LINK style:").grid(row=2, column=0, sticky=tk.W)
        self.cb_link_style = ttk.Combobox(pf, values=["array", "pointer", "pointer_array"], state="readonly", width=14)
        self.cb_link_style.grid(row=2, column=1, sticky=tk.W, padx=4)
        self.cb_link_style.bind("<<ComboboxSelected>>", self._on_pipe_prop_change)

        ttk.Label(pf, text="array_size:").grid(row=3, column=0, sticky=tk.W)
        self.entry_array_size = ttk.Entry(pf, width=8)
        self.entry_array_size.grid(row=3, column=1, sticky=tk.W, padx=4)
        self.entry_array_size.bind("<KeyRelease>", self._on_pipe_prop_change)

        # -- SLOT 调用链条 --
        sf = ttk.LabelFrame(tab, text="SLOT 调用链条", padding=6)
        sf.pack(fill=tk.X, pady=(0, 6))

        columns_chain = ("name", "modules", "comment")
        self.chain_tree = ttk.Treeview(sf, columns=columns_chain, displaycolumns=columns_chain, height=4, selectmode="browse")
        self.chain_tree.pack(fill=tk.X)
        self.chain_tree.heading("name", text="名称")
        self.chain_tree.heading("modules", text="模块")
        self.chain_tree.heading("comment", text="注释")
        self.chain_tree.column("name", width=80)
        self.chain_tree.column("modules", width=200)
        self.chain_tree.column("comment", width=200)

        # -- 字段编辑 --
        ff = ttk.LabelFrame(tab, text="字段列表", padding=6)
        ff.pack(fill=tk.BOTH, expand=True)

        toolbar = ttk.Frame(ff)
        toolbar.pack(fill=tk.X, pady=(0, 4))
        ttk.Button(toolbar, text="+ 字段", command=self.cmd_add_field, width=8).pack(side=tk.LEFT, padx=2)
        ttk.Button(toolbar, text="+ 结构体", command=self.cmd_add_struct_field, width=9).pack(side=tk.LEFT, padx=2)
        ttk.Button(toolbar, text="- 删除", command=self.cmd_delete_field, width=8).pack(side=tk.LEFT, padx=2)

        columns = ("name", "type", "comment")
        self.field_tree = ttk.Treeview(ff, columns=columns, displaycolumns=columns, height=8, selectmode="browse")
        self.field_tree.pack(fill=tk.BOTH, expand=True)
        self.field_tree.heading("name", text="名称")
        self.field_tree.heading("type", text="类型")
        self.field_tree.heading("comment", text="注释")
        self.field_tree.column("name", width=160)
        self.field_tree.column("type", width=180)
        self.field_tree.column("comment", width=200)

        vsb2 = ttk.Scrollbar(ff, orient=tk.VERTICAL, command=self.field_tree.yview)
        vsb2.pack(side=tk.RIGHT, fill=tk.Y)
        self.field_tree.configure(yscrollcommand=vsb2.set)

        # 双击编辑
        self.field_tree.bind("<Double-1>", self._on_field_double_click)

    def _build_pipe_preview_tab(self):
        tab = ttk.Frame(self.notebook)
        self.notebook.add(tab, text="管道预览")

        # 顶部工具栏
        top = ttk.Frame(tab)
        top.pack(fill=tk.X, pady=(2, 4))
        ttk.Button(top, text="确认更新 JSON", command=self._pipe_preview_confirm).pack(side=tk.LEFT, padx=2)
        ttk.Button(top, text="另存为...", command=self._pipe_preview_save_as).pack(side=tk.LEFT, padx=2)
        ttk.Separator(top, orient=tk.VERTICAL).pack(side=tk.LEFT, fill=tk.Y, padx=6)
        ttk.Label(top, text="  (选择管道后显示)", foreground="gray").pack(side=tk.LEFT, padx=4)

        # 左右分割
        pw = ttk.PanedWindow(tab, orient=tk.HORIZONTAL)
        pw.pack(fill=tk.BOTH, expand=True)

        # 左: 原始版 (只读) — 来自 .bak
        left_frame = ttk.LabelFrame(pw, text="原始版本 (.bak)")
        pw.add(left_frame, weight=1)
        self._pipe_preview_old = tk.Text(left_frame, wrap=tk.NONE,
                                         font=("Consolas", 9), bg="#f5f5f5")
        self._pipe_preview_old.pack(fill=tk.BOTH, expand=True)
        self._pipe_preview_old.config(state=tk.DISABLED)
        vsb_po = ttk.Scrollbar(left_frame, orient=tk.VERTICAL,
                               command=self._pipe_preview_old.yview)
        vsb_po.pack(side=tk.RIGHT, fill=tk.Y)
        self._pipe_preview_old.configure(yscrollcommand=vsb_po.set)

        # 右: 可编辑
        right_frame = ttk.LabelFrame(pw, text="新生成 (可编辑 → 确认更新 JSON)")
        pw.add(right_frame, weight=1)
        self._pipe_preview_text = tk.Text(right_frame, wrap=tk.NONE,
                                          font=("Consolas", 9), bg="#fafafa")
        self._pipe_preview_text.pack(fill=tk.BOTH, expand=True)
        vsb_pt = ttk.Scrollbar(right_frame, orient=tk.VERTICAL,
                               command=self._pipe_preview_text.yview)
        vsb_pt.pack(side=tk.RIGHT, fill=tk.Y)
        self._pipe_preview_text.configure(yscrollcommand=vsb_pt.set)

        # 底部水平滚动条（同步）
        hsb_pp = ttk.Scrollbar(tab, orient=tk.HORIZONTAL,
                               command=self._pipe_preview_old.xview)
        hsb_pp.pack(fill=tk.X)
        self._pipe_preview_text.configure(xscrollcommand=hsb_pp.set)
        self._pipe_preview_old.configure(xscrollcommand=hsb_pp.set)

    def _build_code_preview_tab(self):
        tab = ttk.Frame(self.notebook)
        self.notebook.add(tab, text="代码预览")

        # 顶部工具栏
        top = ttk.Frame(tab)
        top.pack(fill=tk.X, pady=(2, 4))
        ttk.Label(top, text="显示:").pack(side=tk.LEFT, padx=4)
        self._code_preview_type = tk.StringVar(value="io.h")
        self._code_preview_cb = ttk.Combobox(top, textvariable=self._code_preview_type,
                                             values=["io.h", ".c", ".h"],
                                             state="readonly", width=8)
        self._code_preview_cb.pack(side=tk.LEFT, padx=4)
        self._code_preview_cb.bind("<<ComboboxSelected>>", self._on_code_preview_type_change)
        self._code_preview_module_label = ttk.Label(top, text="(未选择模块)", foreground="gray")
        self._code_preview_module_label.pack(side=tk.LEFT, padx=6)

        ttk.Separator(top, orient=tk.VERTICAL).pack(side=tk.LEFT, fill=tk.Y, padx=6)
        ttk.Button(top, text="更新 (保存内容)", command=self._code_preview_update).pack(side=tk.LEFT, padx=2)
        ttk.Button(top, text="重新生成", command=self._code_preview_regenerate).pack(side=tk.LEFT, padx=2)
        ttk.Button(top, text="另存为...", command=self._code_preview_save_as).pack(side=tk.LEFT, padx=2)
        ttk.Checkbutton(top, text="显示差异", command=self._code_preview_toggle_diff).pack(side=tk.LEFT, padx=8)
        self._code_preview_diff_var = tk.BooleanVar(value=False)

        # 内容区
        self._code_preview_pw = ttk.PanedWindow(tab, orient=tk.HORIZONTAL)
        self._code_preview_pw.pack(fill=tk.BOTH, expand=True)

        # 左: .bak 差异对比 (默认隐藏)
        self._code_preview_left_frame = ttk.LabelFrame(self._code_preview_pw, text="原始版本 (.bak)")
        self._code_preview_left = tk.Text(self._code_preview_left_frame, wrap=tk.NONE,
                                          font=("Consolas", 9), bg="#f5f5f5")
        self._code_preview_left.pack(fill=tk.BOTH, expand=True)
        self._code_preview_left.config(state=tk.DISABLED)
        self._code_preview_vsb_cl = ttk.Scrollbar(self._code_preview_left_frame, orient=tk.VERTICAL,
                                                  command=self._on_code_vscroll)
        self._code_preview_vsb_cl.pack(side=tk.RIGHT, fill=tk.Y)

        # 右: 可编辑
        right_frame = ttk.LabelFrame(self._code_preview_pw, text="新生成 / 编辑")
        self._code_preview_pw.add(right_frame, weight=1)
        self._code_preview_text = tk.Text(right_frame, wrap=tk.NONE,
                                          font=("Consolas", 9), bg="#fafafa")
        self._code_preview_text.pack(fill=tk.BOTH, expand=True)
        self._code_preview_vsb_cr = ttk.Scrollbar(right_frame, orient=tk.VERTICAL,
                                                  command=self._on_code_vscroll)
        self._code_preview_vsb_cr.pack(side=tk.RIGHT, fill=tk.Y)

        # 两个窗格共享 yscrollcommand（任意一个滚动，两个 scrollbar 同步）
        self._code_preview_left.configure(yscrollcommand=self._on_code_vscroll_set)
        self._code_preview_text.configure(yscrollcommand=self._on_code_vscroll_set)

        # 底部分享水平滚动条
        hsb_cp = ttk.Scrollbar(tab, orient=tk.HORIZONTAL,
                               command=self._on_code_hscroll)
        hsb_cp.pack(fill=tk.X)
        self._code_preview_text.configure(xscrollcommand=hsb_cp.set)
        self._code_preview_left.configure(xscrollcommand=hsb_cp.set)

        # 鼠标滚轮同步
        self._code_preview_left.bind("<MouseWheel>", self._on_code_mousewheel)
        self._code_preview_text.bind("<MouseWheel>", self._on_code_mousewheel)
        self._code_preview_left.bind("<Button-4>", self._on_code_mousewheel)
        self._code_preview_text.bind("<Button-4>", self._on_code_mousewheel)
        self._code_preview_left.bind("<Button-5>", self._on_code_mousewheel)
        self._code_preview_text.bind("<Button-5>", self._on_code_mousewheel)


    def _build_check_tab(self):
        tab = ttk.Frame(self.notebook)
        self.notebook.add(tab, text="检查结果")

        toolbar = ttk.Frame(tab)
        toolbar.pack(fill=tk.X, pady=(0, 4))
        ttk.Button(toolbar, text="▶ 运行检查", command=self.cmd_check).pack(side=tk.LEFT, padx=2)
        ttk.Button(toolbar, text="清空", command=self._clear_check_results).pack(side=tk.LEFT, padx=2)

        # 摘要
        self.lbl_check_summary = ttk.Label(tab, text="(未检查)", anchor=tk.W, relief=tk.SUNKEN, padding=(4, 2))
        self.lbl_check_summary.pack(fill=tk.X, pady=(0, 4))

        # 详细结果
        self.check_text = tk.Text(tab, wrap=tk.NONE, font=("Consolas", 10), bg="#fafafa")
        self.check_text.pack(fill=tk.BOTH, expand=True)
        self.check_text.config(state=tk.DISABLED)

        hsb = ttk.Scrollbar(tab, orient=tk.HORIZONTAL, command=self.check_text.xview)
        hsb.pack(side=tk.BOTTOM, fill=tk.X)
        vsb = ttk.Scrollbar(tab, orient=tk.VERTICAL, command=self.check_text.yview)
        vsb.pack(side=tk.RIGHT, fill=tk.Y)
        self.check_text.configure(xscrollcommand=hsb.set, yscrollcommand=vsb.set)

    # ================================================================
    # 重构工作流 Tab
    # ================================================================
    def _build_refactor_tab(self):
        tab = ttk.Frame(self.notebook)
        self.notebook.add(tab, text="重构工作流")

        # 可滚动容器
        canvas = tk.Canvas(tab, borderwidth=0)
        vsb = ttk.Scrollbar(tab, orient=tk.VERTICAL, command=canvas.yview)
        scroll_frame = ttk.Frame(canvas)
        scroll_frame.bind("<Configure>", lambda e: canvas.configure(scrollregion=canvas.bbox("all")))
        canvas.create_window((0, 0), window=scroll_frame, anchor="nw")
        canvas.configure(yscrollcommand=vsb.set)
        canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        vsb.pack(side=tk.RIGHT, fill=tk.Y)

        # ===== Step 1 =====
        s1 = ttk.LabelFrame(scroll_frame, text="Step 1: 打开 KEIL 项目", padding=8)
        s1.pack(fill=tk.X, pady=4, padx=4)

        row1 = ttk.Frame(s1)
        row1.pack(fill=tk.X, pady=2)
        ttk.Button(row1, text="打开 .uvprojx", command=self._ref_cmd_open_project).pack(side=tk.LEFT, padx=2)
        self._ref_keil_path = ttk.Label(row1, text="(未选择)", anchor=tk.W)
        self._ref_keil_path.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=6)

        row2 = ttk.Frame(s1)
        row2.pack(fill=tk.X, pady=2)
        ttk.Label(row2, text="文件夹白名单 (逗号分隔):").pack(side=tk.LEFT)
        self._ref_whitelist = ttk.Entry(row2, width=30)
        self._ref_whitelist.pack(side=tk.LEFT, padx=4)
        self._ref_whitelist.insert(0, "src, app, drv, hal, proto, base_class, core, cfg")

        row3 = ttk.Frame(s1)
        row3.pack(fill=tk.X, pady=2)
        ttk.Button(row3, text="全选", command=self._ref_select_all).pack(side=tk.LEFT, padx=2)
        ttk.Button(row3, text="取消全选", command=self._ref_deselect_all).pack(side=tk.LEFT, padx=2)
        ttk.Label(row3, text="  候选文件:").pack(side=tk.LEFT, padx=4)

        self._ref_file_container = ttk.Frame(s1)
        self._ref_file_container.pack(fill=tk.BOTH, expand=True, pady=2)
        self._ref_file_vars = []  # list of (tk.BooleanVar, file_info_dict)

        # Treeview for file list
        self._ref_file_tree = ttk.Treeview(self._ref_file_container,
            columns=("sel", "group", "path"), displaycolumns=("sel", "group", "path"),
            height=10, selectmode="none")
        self._ref_file_tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        self._ref_file_tree.heading("sel", text="☐", command=self._ref_on_sel_heading_click)
        self._ref_file_tree.heading("group", text="Group")
        self._ref_file_tree.heading("path", text="文件路径")
        self._ref_file_tree.column("sel", width=30, anchor=tk.CENTER)
        self._ref_file_tree.column("group", width=120)
        self._ref_file_tree.column("path", width=400)
        self._ref_file_tree.bind("<ButtonRelease-1>", self._ref_on_file_click)

        fvsb = ttk.Scrollbar(self._ref_file_container, orient=tk.VERTICAL, command=self._ref_file_tree.yview)
        fvsb.pack(side=tk.RIGHT, fill=tk.Y)
        self._ref_file_tree.configure(yscrollcommand=fvsb.set)

        # ===== Step 2 =====
        s2 = ttk.LabelFrame(scroll_frame, text="Step 2: 导出 AI 分析数据", padding=8)
        s2.pack(fill=tk.X, pady=4, padx=4)

        row_s2 = ttk.Frame(s2)
        row_s2.pack(fill=tk.X, pady=2)
        ttk.Button(row_s2, text="导出扫描文档 (.md)", command=self._ref_cmd_export_scan).pack(side=tk.LEFT, padx=2)
        ttk.Button(row_s2, text="导出扫描 JSON", command=self._ref_cmd_export_scan_json).pack(side=tk.LEFT, padx=2)
        self._ref_scan_status = ttk.Label(s2, text="(未导出)", anchor=tk.W)
        self._ref_scan_status.pack(fill=tk.X, pady=2)

        # ===== Step 3 =====
        s3 = ttk.LabelFrame(scroll_frame, text="Step 3: 导入 AI 生成的数据流 (project.json)", padding=8)
        s3.pack(fill=tk.X, pady=4, padx=4)

        row_s3 = ttk.Frame(s3)
        row_s3.pack(fill=tk.X, pady=2)
        ttk.Button(row_s3, text="导入 project.json", command=self._ref_cmd_import_project_json).pack(side=tk.LEFT, padx=2)
        self._ref_project_json_path = ttk.Label(s3, text="(未导入)", anchor=tk.W)
        self._ref_project_json_path.pack(fill=tk.X, pady=2)

        # ===== Step 4 =====
        s4 = ttk.LabelFrame(scroll_frame, text="Step 4: 生成范式代码", padding=8)
        s4.pack(fill=tk.X, pady=4, padx=4)

        row4a = ttk.Frame(s4)
        row4a.pack(fill=tk.X, pady=2)
        ttk.Label(row4a, text="输出目录:").pack(side=tk.LEFT)
        self._ref_output_dir = ttk.Entry(row4a, width=50)
        self._ref_output_dir.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=4)
        ttk.Button(row4a, text="浏览", command=self._ref_cmd_browse_output).pack(side=tk.LEFT)

        self._ref_gen_io = tk.BooleanVar(value=True)
        self._ref_gen_switcher = tk.BooleanVar(value=True)
        self._ref_gen_existing = tk.BooleanVar(value=True)
        self._ref_gen_new = tk.BooleanVar(value=True)

        row4b = ttk.Frame(s4)
        row4b.pack(fill=tk.X, pady=2)
        ttk.Checkbutton(row4b, text="重建 io.h", variable=self._ref_gen_io).pack(side=tk.LEFT, padx=4)
        ttk.Checkbutton(row4b, text="重建 data_switcher.c", variable=self._ref_gen_switcher).pack(side=tk.LEFT, padx=4)
        ttk.Checkbutton(row4b, text="已有模块追加范式 (.refactored.c)", variable=self._ref_gen_existing).pack(side=tk.LEFT, padx=4)
        ttk.Checkbutton(row4b, text="新建模块完整 .c/.h", variable=self._ref_gen_new).pack(side=tk.LEFT, padx=4)

        row4c = ttk.Frame(s4)
        row4c.pack(fill=tk.X, pady=4)
        ttk.Button(row4c, text="▶ 生成重构代码", command=self._ref_cmd_generate).pack(side=tk.LEFT, padx=2)

        self._ref_log = tk.Text(s4, height=10, wrap=tk.WORD, font=("Consolas", 9), bg="#fafafa")
        self._ref_log.pack(fill=tk.BOTH, expand=True, pady=(4, 0))
        self._ref_log.config(state=tk.DISABLED)

        # 数据
        self._ref_all_files = []
        self._ref_filtered_files = []
        self._ref_imported_config = None
        self._ref_last_keil_dir = ""    # 记住上次 KEIL 项目目录
        self._ref_last_export_dir = ""  # 记住上次导出目录

    # ----- 重构辅助 -----
    def _ref_log_append(self, text: str):
        self._ref_log.config(state=tk.NORMAL)
        self._ref_log.insert(tk.END, text + "\n")
        self._ref_log.see(tk.END)
        self._ref_log.config(state=tk.DISABLED)

    def _ref_rebuild_file_list(self):
        """刷新文件列表 — 树状: GROUP (父) + 文件 (子), 各有复选框"""
        for item in self._ref_file_tree.get_children():
            self._ref_file_tree.delete(item)
        self._ref_group_vars = {}   # {group_name: BooleanVar}
        self._ref_file_items = {}   # {iid: (var, file_dict)}
        self._ref_sel_all_state = True  # 列标题"全选/取消"状态

        if not self._ref_filtered_files:
            self._ref_file_tree.insert("", tk.END, values=("", "(无候选文件 — 请解析 KEIL 项目)", ""))
            return

        # 按 GROUP 分组
        groups = {}
        for f in self._ref_filtered_files:
            g = f["group"]
            if g not in groups:
                groups[g] = []
            groups[g].append(f)

        for gname in sorted(groups.keys()):
            flist = groups[gname]
            # GROUP 父节点
            gvar = tk.BooleanVar(value=True)
            g_iid = self._ref_file_tree.insert("", tk.END,
                values=("☑", f"▼ {gname}", f"({len(flist)} 个文件)"),
                tags=("group",))
            self._ref_file_tree.tag_configure("group", font=("", 9, "bold"))
            self._ref_group_vars[gname] = {"var": gvar, "iid": g_iid, "files": flist}

            # 文件子节点
            for f in flist:
                fvar = tk.BooleanVar(value=True)
                fname = f["file_name"]
                f_iid = self._ref_file_tree.insert(g_iid, tk.END,
                    values=("☑", fname, f["rel_path"]),
                    tags=("file",))
                self._ref_file_items[f_iid] = (fvar, f)

        self._ref_file_tree.tag_configure("file_selected", foreground="black")
        self._ref_file_tree.tag_configure("file_deselected", foreground="gray")

    def _ref_on_file_click(self, event):
        """点击切换选中状态 — GROUP 勾选/取消全部，文件只切自身"""
        iid = self._ref_file_tree.identify_row(event.y)
        if not iid:
            return

        # 点击展开/折叠图标(▶/▼)时不切换选中状态
        region = self._ref_file_tree.identify_region(event.x, event.y)
        if region == "tree":
            return

        # 判断是 GROUP 还是 文件
        tags = self._ref_file_tree.item(iid, "tags")

        if "group" in tags:
            # GROUP 节点: 切换自身 + 所有子文件
            gname = self._ref_file_tree.item(iid, "values")[1].replace("▼ ", "", 1)
            ginfo = self._ref_group_vars.get(gname)
            if not ginfo:
                return
            new_val = not ginfo["var"].get()
            ginfo["var"].set(new_val)
            self._ref_file_tree.set(iid, "sel", "☑" if new_val else "☐")
            self._ref_file_tree.item(iid, tags=("group",))

            # 更新所有子文件
            for child_iid in self._ref_file_tree.get_children(iid):
                if child_iid in self._ref_file_items:
                    fvar, f = self._ref_file_items[child_iid]
                    fvar.set(new_val)
                    self._ref_file_tree.set(child_iid, "sel", "☑" if new_val else "☐")
                    tag = ("file", "file_selected") if new_val else ("file", "file_deselected")
                    self._ref_file_tree.item(child_iid, tags=tag)

        elif "file" in tags:
            if iid not in self._ref_file_items:
                return
            fvar, f = self._ref_file_items[iid]
            new_val = not fvar.get()
            fvar.set(new_val)
            self._ref_file_tree.set(iid, "sel", "☑" if new_val else "☐")
            tag = ("file", "file_selected") if new_val else ("file", "file_deselected")
            self._ref_file_tree.item(iid, tags=tag)

        # 刷新 GROUP 显示状态 (☑全选 ☐全不选 ☒部分选)
        self._ref_update_group_state()

    def _ref_on_sel_heading_click(self):
        """点击有效列标题 → 切换全选/取消全选"""
        self._ref_sel_all_state = not self._ref_sel_all_state
        if self._ref_sel_all_state:
            self._ref_select_all()
            self._ref_file_tree.heading("sel", text="☑")
        else:
            self._ref_deselect_all()
            self._ref_file_tree.heading("sel", text="☐")

    def _ref_select_all(self):
        """全选所有 GROUP"""
        for gname, ginfo in self._ref_group_vars.items():
            ginfo["var"].set(True)
            self._ref_file_tree.set(ginfo["iid"], "sel", "☑")
            for child_iid in self._ref_file_tree.get_children(ginfo["iid"]):
                if child_iid in self._ref_file_items:
                    fvar, f = self._ref_file_items[child_iid]
                    fvar.set(True)
                    self._ref_file_tree.set(child_iid, "sel", "☑")
                    self._ref_file_tree.item(child_iid, tags=("file", "file_selected"))
        self._ref_update_group_state()

    def _ref_deselect_all(self):
        """取消全选所有 GROUP"""
        for gname, ginfo in self._ref_group_vars.items():
            ginfo["var"].set(False)
            self._ref_file_tree.set(ginfo["iid"], "sel", "☐")
            for child_iid in self._ref_file_tree.get_children(ginfo["iid"]):
                if child_iid in self._ref_file_items:
                    fvar, f = self._ref_file_items[child_iid]
                    fvar.set(False)
                    self._ref_file_tree.set(child_iid, "sel", "☐")
                    self._ref_file_tree.item(child_iid, tags=("file", "file_deselected"))
        self._ref_update_group_state()

    def _ref_get_selected_files(self) -> list:
        """返回所有选中文件 (文件 BooleanVar 是唯一判断依据)"""
        result = []
        for iid, (fvar, f) in self._ref_file_items.items():
            if fvar.get():
                result.append(f)
        return result

    def _ref_update_group_state(self):
        """刷新所有 GROUP 的显示状态: ☑全选 ☐全不选 ☒部分选"""
        for gname, ginfo in self._ref_group_vars.items():
            g_iid = ginfo["iid"]
            child_vars = []
            for cid in self._ref_file_tree.get_children(g_iid):
                if cid in self._ref_file_items:
                    cv, _ = self._ref_file_items[cid]
                    child_vars.append(cv.get())
            if not child_vars:
                continue
            all_sel = all(child_vars)
            none_sel = not any(child_vars)
            if all_sel:
                self._ref_file_tree.set(g_iid, "sel", "☑")
                self._ref_file_tree.item(g_iid, tags=("group",))
            elif none_sel:
                self._ref_file_tree.set(g_iid, "sel", "☐")
                self._ref_file_tree.item(g_iid, tags=("group",))
            else:
                self._ref_file_tree.set(g_iid, "sel", "☒")

    # ----- Step 1 -----
    def _ref_cmd_open_project(self):
        path = filedialog.askopenfilename(
            filetypes=[("KEIL Project", "*.uvprojx *.uvproj"), ("All", "*.*")],
            title="打开 KEIL 项目文件",
            initialdir=self._ref_last_keil_dir or None)
        if not path:
            return
        self._ref_last_keil_dir = os.path.dirname(path)
        self._ref_keil_path.config(text=path)
        self._ref_log_append(f"[INFO] 解析 KEIL 项目: {path}")
        try:
            whitelist_str = self._ref_whitelist.get().strip()
            whitelist = [w.strip() for w in whitelist_str.split(",") if w.strip()] or None
            all_files = parse_uvprojx(path)
            self._ref_all_files = all_files
            self._ref_log_append(f"  共 {len(all_files)} 个源文件")
            filtered = filter_files(all_files, folder_whitelist=whitelist)
            self._ref_filtered_files = filtered
            self._ref_log_append(f"  白名单 '{whitelist}' 过滤后: {len(filtered)} 个候选")
            self._ref_rebuild_file_list()
            self._ref_log_append("[OK] 文件列表已加载，勾选要重构的文件")
            proj_dir = os.path.dirname(path)
            parent = os.path.dirname(proj_dir)
            self._ref_output_dir.delete(0, tk.END)
            self._ref_output_dir.insert(0, os.path.join(parent, "src2"))
        except Exception as e:
            self._ref_log_append(f"[ERROR] {e}")
            messagebox.showerror("解析失败", str(e))

    # ----- Step 2 -----
    def _ref_cmd_export_scan(self):
        selected = self._ref_get_selected_files()
        if not selected:
            messagebox.showinfo("提示", "请先勾选源文件")
            return
        scan_data = scan_source_files(selected)
        if not scan_data["files"]:
            messagebox.showinfo("提示", "没有 .c 文件可扫描")
            return
        path = filedialog.asksaveasfilename(defaultextension=".md",
            filetypes=[("Markdown", "*.md"), ("All", "*.*")], initialfile="ai_scan_doc.md",
            initialdir=self._ref_last_export_dir or None)
        if not path:
            return
        self._ref_last_export_dir = os.path.dirname(path)
        doc = generate_ai_scan_doc(scan_data, path)
        self._ref_scan_status.config(text=f"已导出: {path}")
        self._ref_log_append(f"[OK] AI 分析文档: {path} ({len(doc)} 字符)")
        messagebox.showinfo("导出完成",
            f"包含 {len(scan_data['files'])} 个文件的函数签名/结构体/依赖信息。\n"
            f"将文档提供给 AI 分析数据流向并生成 project.json。")

    def _ref_cmd_export_scan_json(self):
        selected = self._ref_get_selected_files()
        if not selected:
            messagebox.showinfo("提示", "请先勾选源文件")
            return
        scan_data = scan_source_files(selected)
        path = filedialog.asksaveasfilename(defaultextension=".json",
            filetypes=[("JSON", "*.json"), ("All", "*.*")], initialfile="ai_scan_data.json",
            initialdir=self._ref_last_export_dir or None)
        if not path:
            return
        self._ref_last_export_dir = os.path.dirname(path)
        import json as j
        with open(path, "w", encoding="utf-8") as f:
            j.dump(scan_data, f, ensure_ascii=False, indent=2)
        self._ref_scan_status.config(text=f"已导出: {path}")
        self._ref_log_append(f"[OK] 扫描 JSON: {path}")

    # ----- Step 3 -----
    def _ref_cmd_import_project_json(self):
        path = filedialog.askopenfilename(
            filetypes=[("JSON", "*.json"), ("All", "*.*")], title="导入 project.json")
        if not path:
            return
        try:
            import json as j
            with open(path, "r", encoding="utf-8") as f:
                config = j.load(f)
            if "modules" not in config or "pipes" not in config:
                raise ValueError("缺少 modules 或 pipes 段")
            self._ref_imported_config = config
            self._ref_project_json_path.config(text=path)
            self._ref_log_append(f"[OK] project.json 已导入: {len(config['modules'])} 模块, {len(config['pipes'])} 管道")
            self._load_config_data(config, path)
            # 同步更新重构面板的输出目录（确保绝对路径，从盘符开始）
            raw_root = config.get("project", {}).get("output_root", "")
            if raw_root:
                if not os.path.isabs(raw_root):
                    raw_root = os.path.abspath(os.path.join(os.path.dirname(path), raw_root))
                self._ref_output_dir.delete(0, tk.END)
                self._ref_output_dir.insert(0, raw_root)
        except Exception as e:
            self._ref_log_append(f"[ERROR] {e}")
            messagebox.showerror("导入失败", str(e))

    # ----- Step 4 -----
    def _ref_cmd_browse_output(self):
        path = filedialog.askdirectory(title="选择输出目录")
        if path:
            self._ref_output_dir.delete(0, tk.END)
            self._ref_output_dir.insert(0, path)

    def _ref_cmd_generate(self):
        config = self._ref_imported_config
        if not config:
            messagebox.showinfo("提示", "请先导入 project.json (Step 3)")
            return
        output_root = self._ref_output_dir.get().strip()
        if not output_root:
            messagebox.showerror("错误", "请填写输出目录")
            return

        # 备份已有输出目录
        if os.path.isdir(output_root):
            import shutil, time
            backup_dir = output_root.rstrip("/\\") + f".backup_{time.strftime('%Y%m%d_%H%M%S')}"
            try:
                shutil.copytree(output_root, backup_dir)
                self._ref_log_append(f"[备份] 已备份原目录 → {backup_dir}")
            except Exception as e:
                self._ref_log_append(f"[警告] 备份失败: {e}")

        project = config.get("project", {})
        paths = project.get("paths", {})
        modules = config.get("modules", [])
        pipes = config.get("pipes", [])
        slot_order = config.get("slot_order", [m["name"] for m in modules])
        slot_chains = config.get("slot_chains", [])
        io_dir = paths.get("io_dir", "include")
        core_dir = paths.get("core_dir", "core")

        self._ref_log_append("")
        self._ref_log_append("=" * 50)
        self._ref_log_append("生成重构代码...")
        self._ref_log_append(f"  输出: {output_root}")

        try:
            generated = []

            # 1. io.h
            if self._ref_gen_io.get():
                io_out = os.path.join(output_root, io_dir)
                os.makedirs(io_out, exist_ok=True)
                for mod in modules:
                    c = generate_io_h(mod, pipes, project)
                    fp = os.path.join(io_out, f"{_pascal_to_snake(mod['name'])}_io.h")
                    with open(fp, "w", encoding="utf-8") as f:
                        f.write(c)
                    generated.append(fp)
                self._ref_log_append(f"  [io.h] {len(modules)} 个")

            # 2. data_switcher.c
            if self._ref_gen_switcher.get():
                core_out = os.path.join(output_root, core_dir)
                os.makedirs(core_out, exist_ok=True)
                c = generate_switcher(modules, pipes, slot_order, project, slot_chains)
                fp = os.path.join(core_out, "data_switcher.c")
                with open(fp, "w", encoding="utf-8") as f:
                    f.write(c)
                generated.append(fp)
                self._ref_log_append("  [switcher] data_switcher.c")

            # 3. 已有模块追加范式
            if self._ref_gen_existing.get():
                self._ref_log_append("  [已有模块] 追加范式到原文件尾...")
                for mod in modules:
                    mod_name = mod["name"]
                    src = mod.get("source_file", f"{mod['layer']}/{mod_name.lower()}.c")
                    orig_path = ""
                    for sf in self._ref_filtered_files:
                        if mod_name.lower() in sf["rel_path"].lower().replace("_", ""):
                            orig_path = sf["abs_path"]
                            break
                    if not orig_path:
                        proj_dir = os.path.dirname(self._ref_keil_path.cget("text"))
                        cand = os.path.normpath(os.path.join(proj_dir, "..", src))
                        if os.path.isfile(cand):
                            orig_path = cand
                    if not orig_path or not os.path.isfile(orig_path):
                        self._ref_log_append(f"    ⚠ {mod_name}: 原文件未找到, 跳过")
                        continue
                    c = generate_module_c_refactored(mod, pipes, project, orig_path)
                    mod_out = os.path.join(output_root, mod["layer"])
                    os.makedirs(mod_out, exist_ok=True)
                    fp = os.path.join(mod_out, f"{_pascal_to_snake(mod_name)}.refactored.c")
                    with open(fp, "w", encoding="utf-8") as f:
                        f.write(c)
                    generated.append(fp)
                    self._ref_log_append(f"    ✓ {mod_name}")

            # 4. 新建模块
            if self._ref_gen_new.get():
                for mod in modules:
                    mn = mod["name"]
                    ml = mn.lower()
                    is_new = all(ml not in sf["rel_path"].lower().replace("_", "")
                                 for sf in self._ref_filtered_files)
                    if not is_new:
                        continue
                    mod_out = os.path.join(output_root, mod["layer"])
                    os.makedirs(mod_out, exist_ok=True)
                    cc = generate_module_c(mod, pipes, project)
                    with open(os.path.join(mod_out, f"{ml}.c"), "w", encoding="utf-8") as f:
                        f.write(cc)
                    hc = generate_module_h(mod)
                    with open(os.path.join(mod_out, f"{ml}.h"), "w", encoding="utf-8") as f:
                        f.write(hc)
                    generated.append(mod_out)
                    self._ref_log_append(f"    ✚ {mn}: 新建模块文件")

            self._ref_log_append(f"\n[完成] 共 {len(generated)} 个文件")

        except Exception as e:
            self._ref_log_append(f"[ERROR] {e}")
            messagebox.showerror("生成失败", str(e))

    # ================================================================
    # 底部按钮栏
    # ================================================================
    def _build_bottom_bar(self):
        bar = ttk.Frame(self.root)
        bar.pack(fill=tk.X, padx=4, pady=(0, 4))

        ttk.Button(bar, text="打开 JSON", command=self.cmd_open).pack(side=tk.LEFT, padx=2)
        ttk.Button(bar, text="保存 JSON", command=self.cmd_save).pack(side=tk.LEFT, padx=2)
        ttk.Separator(bar, orient=tk.VERTICAL).pack(side=tk.LEFT, fill=tk.Y, padx=6)
        ttk.Button(bar, text="检查", command=self.cmd_check).pack(side=tk.LEFT, padx=2)
        ttk.Separator(bar, orient=tk.VERTICAL).pack(side=tk.LEFT, fill=tk.Y, padx=6)
        ttk.Button(bar, text="全部生成", command=lambda: self.cmd_generate("all")).pack(side=tk.LEFT, padx=2)
        ttk.Button(bar, text="仅 io.h", command=lambda: self.cmd_generate("io")).pack(side=tk.LEFT, padx=2)
        ttk.Button(bar, text="仅 switcher", command=lambda: self.cmd_generate("switcher")).pack(side=tk.LEFT, padx=2)
        ttk.Button(bar, text="仅 modules", command=lambda: self.cmd_generate("modules")).pack(side=tk.LEFT, padx=2)
        ttk.Button(bar, text="⟳ 强制刷新", command=self.cmd_force_refresh).pack(side=tk.LEFT, padx=2)

        self.lbl_status = ttk.Label(bar, relief=tk.SUNKEN, anchor=tk.W, padding=(4, 2))
        self.lbl_status.pack(side=tk.RIGHT, fill=tk.X, expand=True, padx=(12, 0))

    def status(self, msg):
        self.lbl_status.config(text=msg)

    # ================================================================
    # 数据加载 / 保存
    # ================================================================
    def _load_config_data(self, data: dict, path: str = None):
        """从 dict 加载配置到 UI"""
        self.config = data
        self.config_path = path
        self.modules = data.get("modules", [])
        self.pipes = data.get("pipes", [])
        self.slot_order = data.get("slot_order", [m["name"] for m in self.modules])

        # 填充链条
        self.chain_tree.delete(*self.chain_tree.get_children())
        for ch in data.get("slot_chains", []):
            self.chain_tree.insert("", tk.END, values=(ch["name"], " → ".join(ch["modules"]), ch.get("comment", "")))

        # 填充项目设置
        proj = data.get("project", {})
        paths = proj.get("paths", {})
        raw_root = proj.get("output_root", "")
        # 相对路径 → 解析为 JSON 所在目录的绝对路径
        if path and raw_root and not os.path.isabs(raw_root):
            raw_root = os.path.abspath(os.path.join(os.path.dirname(path), raw_root))
        self.entry_output_root.delete(0, tk.END)
        self.entry_output_root.insert(0, raw_root)
        for key in ("io_dir", "core_dir", "app_dir", "base_class_dir", "proto_dir"):
            ent = getattr(self, f"entry_{key.replace('.', '_')}", None)
            if ent:
                ent.delete(0, tk.END)
                ent.insert(0, paths.get(key, ""))
        self.entry_pack.delete(0, tk.END)
        self.entry_pack.insert(0, str(proj.get("pack", 4)))

        # 填充模块树
        self._rebuild_tree()
        self._clear_pipe_editor()
        self._update_code_preview("")
        self.status(f"已加载: {path or '内存数据'}")

    def _collect_config(self) -> dict:
        """从 UI 控件收集当前配置 → dict"""
        paths = {
            "io_dir": self.entry_io_dir.get(),
            "core_dir": self.entry_core_dir.get(),
            "app_dir": self.entry_app_dir.get(),
            "base_class_dir": self.entry_base_class_dir.get(),
            "proto_dir": self.entry_proto_dir.get(),
        }
        project = {
            "name": self.config.get("project", {}).get("name", "IH_Project"),
            "paths": paths,
            "std_module_path": self.config.get("project", {}).get("std_module_path", "core/std_module.h"),
            "pack": int(self.entry_pack.get() or 4),
            "comment_guard": self.config.get("project", {}).get("comment_guard", True),
            "output_root": os.path.abspath(self.entry_output_root.get()),
        }
        # 从链条树收集
        slot_chains = []
        for child in self.chain_tree.get_children():
            vals = self.chain_tree.item(child, "values")
            if len(vals) >= 2:
                name = vals[0]
                modules = [m.strip() for m in vals[1].split("→")]
                comment = vals[2] if len(vals) > 2 else ""
                slot_chains.append({"name": name, "modules": modules, "comment": comment})

        return {
            "schema_version": "1.0",
            "project": project,
            "modules": self.modules,
            "pipes": self.pipes,
            "slot_order": self.slot_order,
            "slot_chains": slot_chains,
        }

    def cmd_open(self):
        path = filedialog.askopenfilename(
            filetypes=[("JSON", "*.json"), ("All", "*.*")],
            title="打开项目配置")
        if not path:
            return
        try:
            with open(path, "r", encoding="utf-8") as f:
                data = json.load(f)
            self._load_config_data(data, path)
        except Exception as e:
            messagebox.showerror("打开失败", str(e))

    def cmd_save(self):
        if not self.config_path:
            self.cmd_save_as()
            return
        self._write_config(self.config_path)

    def cmd_save_as(self):
        path = filedialog.asksaveasfilename(
            defaultextension=".json",
            filetypes=[("JSON", "*.json"), ("All", "*.*")],
            title="另存为")
        if not path:
            return
        self._write_config(path)
        self.config_path = path
        self.status(f"已保存: {path}")

    def _write_config(self, path):
        """将 UI 状态写回 JSON 文件"""
        # 先收集项目设置中的修改
        data = self._collect_config()
        try:
            with open(path, "w", encoding="utf-8") as f:
                json.dump(data, f, ensure_ascii=False, indent=2)
            self.status(f"已保存: {path}")
        except Exception as e:
            messagebox.showerror("保存失败", str(e))

    # ================================================================
    # 模块树管理
    # ================================================================
    def _rebuild_tree(self):
        """根据 self.modules/pipes 重建树"""
        self.tree.delete(*self.tree.get_children())
        self.field_tree.delete(*self.field_tree.get_children())

        mod_map = {m["name"]: m for m in self.modules}

        for mod in self.modules:
            mname = mod["name"]
            node_id = self.tree.insert("", tk.END, text=mname, values=("module",), open=True)

            # 入参
            in_pipes = [p for p in self.pipes if p["to"] == mname]
            if in_pipes:
                in_node = self.tree.insert(node_id, tk.END, text="入参", values=("in",), open=True)
                for p in in_pipes:
                    label = f"◀ {p['from']}"
                    self.tree.insert(in_node, tk.END, text=label, values=("pipe", p["id"]))
            else:
                self.tree.insert(node_id, tk.END, text="(无入参)", values=("none",))

            # 出参
            out_pipes = [p for p in self.pipes if p["from"] == mname]
            if out_pipes:
                out_node = self.tree.insert(node_id, tk.END, text="出参", values=("out",), open=True)
                for p in out_pipes:
                    label = f"▶ {p['to']}"
                    self.tree.insert(out_node, tk.END, text=label, values=("pipe", p["id"]))
            else:
                self.tree.insert(node_id, tk.END, text="(无出参)", values=("none",))

        # data_switcher 节点（无管道声明，但需代码预览）
        self.tree.insert("", tk.END, text="[data_switcher]", values=("switcher",), open=True)

    def _on_tree_select(self, event):
        sel = self.tree.selection()
        if not sel:
            return
        item = sel[0]
        vals = self.tree.item(item, "values")
        if not vals:
            self._clear_pipe_editor()
            return

        val_type = vals[0]

        if val_type == "pipe":
            # 管道选中 → 管道预览 + 管道编辑
            pipe_id = vals[1]
            pipe = next((p for p in self.pipes if p["id"] == pipe_id), None)
            if not pipe:
                return
            self._selected_pipe = pipe
            self._show_pipe(pipe)
            self._refresh_field_table(pipe)
            self._show_pipe_preview(pipe)

        elif val_type == "switcher":
            # data_switcher 预览
            self._clear_pipe_editor()
            self._show_data_switcher_preview()

        else:
            self._clear_pipe_editor()
            # 找最近的模块节点 (自身或父节点)
            mod_name = self._find_module_node(item)
            if mod_name:
                self._show_module_preview_by_name(mod_name)

    def _find_module_node(self, item) -> str:
        """从树节点向上找模块名"""
        vals = self.tree.item(item, "values")
        if vals and vals[0] == "module":
            return self.tree.item(item, "text")
        parent = self.tree.parent(item)
        if parent:
            return self._find_module_node(parent)
        return ""

    def _show_module_preview_by_name(self, module_name: str):
        """按模块名生成代码预览"""
        mod = next((m for m in self.modules if m["name"] == module_name), None)
        if not mod:
            self._update_code_preview(f"// 模块 {module_name} 未找到")
            return
        self._code_preview_module_label.config(text=module_name)
        self._show_module_preview(mod, self._code_preview_type.get())

    def _show_module_preview(self, mod: dict, file_type: str = "io.h"):
        """生成模块指定文件类型的代码"""
        config = self._collect_config()
        project = config.get("project", {})
        paths = project.get("paths", {})
        pipes = config.get("pipes", [])
        output_root = self.entry_output_root.get().strip()

        # 构造文件路径 (绝对路径, 用于 .bak 比对和写入)
        mod_name = _pascal_to_snake(mod["name"])
        if file_type == "io.h":
            io_dir = paths.get("io_dir", "include")
            file_path = os.path.abspath(os.path.join(output_root, io_dir, f"{mod_name}_io.h"))
        elif file_type in (".c", ".h"):
            source_file = mod.get("source_file", "")
            if source_file:
                if file_type == ".c":
                    file_path = os.path.abspath(os.path.join(output_root, source_file))
                else:
                    file_path = os.path.abspath(os.path.join(output_root,
                                                os.path.splitext(source_file)[0] + ".h"))
            else:
                layer_dir = paths.get(f"{mod['layer']}_dir", mod['layer'])
                file_path = os.path.abspath(os.path.join(output_root, layer_dir,
                                            f"{mod_name}{file_type}"))
        else:
            layer_dir = paths.get(f"{mod['layer']}_dir", mod['layer'])
            file_path = os.path.abspath(os.path.join(output_root, layer_dir, f"{mod_name}{file_type}"))

        try:
            if file_type == "io.h":
                content = generate_io_h(mod, pipes, project)
            elif file_type == ".c":
                # 已有文件用重构模式（保留原内容，插 AI 段到最前面）
                if os.path.isfile(file_path):
                    content = generate_module_c_refactored(mod, pipes, project, file_path)
                else:
                    content = generate_module_c(mod, pipes, project)
            elif file_type == ".h":
                content = generate_module_h(mod)
            else:
                content = f"// 未知文件类型: {file_type}"
            self._update_code_preview(content, file_path)
            self.status(f"预览: {mod['name']} ({file_type}) → {file_path}")
        except Exception as e:
            self._update_code_preview(f"// 生成错误: {e}")

    def _on_code_preview_type_change(self, event=None):
        """代码预览文件类型切换"""
        module_name = self._code_preview_module_label.cget("text")
        if module_name and module_name != "(未选择模块)":
            mod = next((m for m in self.modules if m["name"] == module_name), None)
            if mod:
                self._show_module_preview(mod, self._code_preview_type.get())

    def cmd_add_module(self):
        dlg = ModuleDialog(self.root, self.modules)
        if not dlg.result:
            return
        name, layer, comment = dlg.result
        # 检查重名
        if any(m["name"] == name for m in self.modules):
            messagebox.showerror("错误", f"模块 {name} 已存在")
            return
        new_mod = {
            "id": name.lower(),
            "name": name,
            "layer": layer,
            "source_file": f"{layer}/{name.lower()}.c",
            "comment": comment,
        }
        self.modules.append(new_mod)
        if name not in self.slot_order:
            self.slot_order.append(name)
        self._rebuild_tree()
        self.status(f"已添加模块: {name}")

    def cmd_delete_module(self):
        sel = self.tree.selection()
        if not sel:
            return
        item = sel[0]
        vals = self.tree.item(item, "values")
        if not vals or vals[0] == "pipe":
            messagebox.showinfo("提示", "请选中模块节点 (不是管道)")
            return
        # 找到模块节点
        while vals and vals[0] == "pipe":
            item = self.tree.parent(item)
            vals = self.tree.item(item, "values")
        if not vals or vals[0] != "module":
            return
        mname = self.tree.item(item, "text")

        if not messagebox.askyesno("确认", f"删除模块 {mname} 及其所有管道?"):
            return

        self.modules = [m for m in self.modules if m["name"] != mname]
        self.pipes = [p for p in self.pipes if p["from"] != mname and p["to"] != mname]
        if mname in self.slot_order:
            self.slot_order.remove(mname)
        self._selected_pipe = None
        self._rebuild_tree()
        self._clear_pipe_editor()
        self.status(f"已删除模块: {mname}")

    def cmd_delete_pipe(self):
        sel = self.tree.selection()
        if not sel:
            return
        vals = self.tree.item(sel[0], "values")
        if not vals or vals[0] != "pipe":
            messagebox.showinfo("提示", "请选中管道节点")
            return
        pipe_id = vals[1]
        pipe = next((p for p in self.pipes if p["id"] == pipe_id), None)
        if not pipe:
            return
        if not messagebox.askyesno("确认", f"删除管道 {pipe['from']} → {pipe['to']}?"):
            return
        self.pipes = [p for p in self.pipes if p["id"] != pipe_id]
        self._selected_pipe = None
        self._rebuild_tree()
        self._clear_pipe_editor()
        self.status(f"已删除管道: {pipe['from']} → {pipe['to']}")

    def cmd_add_pipe(self):
        """新增管道对话框"""
        mod_names = [m["name"] for m in self.modules]
        if len(mod_names) < 2:
            messagebox.showinfo("提示", "至少需要 2 个模块才能创建管道")
            return
        dlg = PipeDialog(self.root, mod_names, self.pipes)
        if not dlg.result:
            return
        from_name, to_name, callback_type = dlg.result
        # 检查重复
        for p in self.pipes:
            if p["from"] == from_name and p["to"] == to_name:
                messagebox.showerror("错误", f"管道 {from_name} → {to_name} 已存在")
                return
        pipe_id = f"pipe_{from_name.lower()}_{to_name.lower()}"
        new_pipe = {
            "id": pipe_id,
            "from": from_name,
            "to": to_name,
            "callback_type": callback_type,
            "out_link": {"style": "array", "array_size": 4},
            "in_link": {"style": "array", "array_size": 4},
            "fields": [],
            "comment": "",
        }
        self.pipes.append(new_pipe)
        self._rebuild_tree()
        self.status(f"已添加管道: {from_name} → {to_name}")

    # ================================================================
    # 管道编辑
    # ================================================================
    def _clear_pipe_editor(self):
        self._selected_pipe = None
        self.lbl_pipe_name.config(text="(未选择)")
        self.cb_callback_type.set("")
        self.cb_link_style.set("")
        self.entry_array_size.delete(0, tk.END)
        self.field_tree.delete(*self.field_tree.get_children())
        self._update_pipe_preview("")

    def _show_pipe(self, pipe):
        self.lbl_pipe_name.config(text=f"{pipe['from']} → {pipe['to']}")
        self.cb_callback_type.set(pipe.get("callback_type", "pull"))
        out_style = pipe.get("out_link", {}).get("style", "array")
        self.cb_link_style.set(out_style)
        arr_size = pipe.get("out_link", {}).get("array_size", "")
        self.entry_array_size.delete(0, tk.END)
        if arr_size:
            self.entry_array_size.insert(0, str(arr_size))

    def _on_pipe_prop_change(self, event=None):
        if not self._selected_pipe:
            return
        p = self._selected_pipe
        p["callback_type"] = self.cb_callback_type.get()

        style = self.cb_link_style.get()
        if "out_link" not in p:
            p["out_link"] = {}
        p["out_link"]["style"] = style
        if style == "array":
            try:
                p["out_link"]["array_size"] = int(self.entry_array_size.get() or 4)
            except ValueError:
                pass
        elif "array_size" in p["out_link"]:
            del p["out_link"]["array_size"]

        # 保持 in_link 与 out_link 同步
        p["in_link"] = dict(p["out_link"])

        self._show_pipe_preview(p)
        self.status(f"已更新管道: {p['from']} → {p['to']}")

    # ================================================================
    # 字段编辑
    # ================================================================
    def _find_type_definition(self, type_name: str) -> dict | None:
        """在全部模块的 types[] 中查找外部类型定义"""
        for mod in self.modules:
            for t in mod.get("types", []):
                if t.get("name") == type_name:
                    return t
        return None

    def _refresh_field_table(self, pipe):
        self._field_item_map.clear()
        self.field_tree.delete(*self.field_tree.get_children())
        for f in pipe.get("fields", []):
            self._populate_field_tree("", f)

    def _populate_field_tree(self, parent, field):
        """递归填充字段 Treeview（支持嵌套结构体和外部类型引用）"""
        name = field.get("name", "")
        ftype = field.get("type", "")
        comment = field.get("comment", "")

        if ftype == "struct" and field.get("fields"):
            iid = self.field_tree.insert(parent, tk.END, values=(f"▸ {name}", "struct", comment), open=True)
            self._field_item_map[iid] = field
            for sub in field["fields"]:
                self._populate_field_tree(iid, sub)
        else:
            # 检查是否是指向外部类型 (types[]) 的指针字段
            base_type = ftype.rstrip('*').strip()
            type_def = self._find_type_definition(base_type) if base_type and base_type != ftype else None
            if type_def:
                iid = self.field_tree.insert(parent, tk.END, values=(f"▸ {name}", ftype, comment), open=True)
                self._field_item_map[iid] = field
                for sub in type_def.get("fields", []):
                    self._populate_field_tree(iid, sub)
            else:
                iid = self.field_tree.insert(parent, tk.END, values=(name, ftype, comment))
                self._field_item_map[iid] = field

    def _remove_field_recursive(self, fields, target_field):
        """递归查找并移除目标字段（支持嵌套）"""
        for i, f in enumerate(fields):
            if f is target_field:
                fields.pop(i)
                return True
            if f.get("fields"):
                if self._remove_field_recursive(f["fields"], target_field):
                    if not f["fields"]:  # 结构体变空时也移除
                        fields.pop(i)
                    return True
        return False

    def cmd_add_field(self):
        if not self._selected_pipe:
            messagebox.showinfo("提示", "请先选择一个管道")
            return
        fields = self._selected_pipe.setdefault("fields", [])
        fields.append({"name": "new_field", "type": "uint16_t", "comment": ""})
        self._refresh_field_table(self._selected_pipe)
        self._show_pipe_preview(self._selected_pipe)

    def cmd_add_struct_field(self):
        if not self._selected_pipe:
            messagebox.showinfo("提示", "请先选择一个管道")
            return
        fields = self._selected_pipe.setdefault("fields", [])
        fields.append({
            "name": "new_struct",
            "type": "struct",
            "comment": "",
            "fields": [
                {"name": "field1", "type": "uint16_t", "comment": ""}
            ]
        })
        self._refresh_field_table(self._selected_pipe)
        self._show_pipe_preview(self._selected_pipe)

    def cmd_delete_field(self):
        sel = self.field_tree.selection()
        if not sel or not self._selected_pipe:
            return
        iid = sel[0]
        field = self._field_item_map.get(iid)
        if not field:
            return
        if not messagebox.askyesno("确认", f"删除字段 '{field.get('name', '?')}'?"):
            return
        fields = self._selected_pipe.get("fields", [])
        self._remove_field_recursive(fields, field)
        self._refresh_field_table(self._selected_pipe)
        self._show_pipe_preview(self._selected_pipe)

    def _on_field_double_click(self, event):
        if not self._selected_pipe:
            return
        sel = self.field_tree.selection()
        if not sel:
            return
        iid = sel[0]
        field = self._field_item_map.get(iid)
        if not field:
            return

        # 有子节点的字段行: 展开/折叠 (支持嵌套结构体和外部类型引用)
        if self.field_tree.get_children(iid):
            was_open = self.field_tree.item(iid, "open")
            self.field_tree.item(iid, open=not was_open)
            return

        col = self.field_tree.identify_column(event.x)
        col_idx = int(col.replace("#", "")) - 1  # 0=name, 1=type, 2=comment

        item = sel[0]
        x, y, w, h = self.field_tree.bbox(item, col)

        key = ["name", "type", "comment"][col_idx]
        old_val = self.field_tree.set(item, col) or ""

        entry = ttk.Entry(self.field_tree, width=w // 8 + 4)
        entry.place(x=x, y=y, w=w, h=h)
        entry.insert(0, old_val)
        entry.focus_set()
        entry.selection_range(0, tk.END)

        def _on_confirm(event=None):
            new_val = entry.get()
            entry.destroy()
            self.field_tree.set(item, col, new_val)
            # 更新内存数据
            field[key] = new_val
            # 如果从 struct 改回普通类型，清理子字段
            if key == "type" and new_val != "struct" and "fields" in field:
                del field["fields"]
                self._refresh_field_table(self._selected_pipe)
            self._show_pipe_preview(self._selected_pipe)

        def _on_cancel(event=None):
            entry.destroy()

        entry.bind("<Return>", _on_confirm)
        entry.bind("<Escape>", _on_cancel)
        entry.bind("<FocusOut>", _on_confirm)

    # ================================================================
    # 代码预览 — 滚动同步
    # ================================================================
    def _on_code_vscroll(self, *args):
        """垂直滚动 → 同步两个 Text 控件"""
        self._code_preview_left.yview(*args)
        self._code_preview_text.yview(*args)

    def _on_code_vscroll_set(self, *args):
        """两个 Text 控件的 yscrollcommand → 同步两个 scrollbar"""
        self._code_preview_vsb_cl.set(*args)
        self._code_preview_vsb_cr.set(*args)

    def _on_code_hscroll(self, *args):
        """水平滚动 → 同步两个 Text 控件"""
        self._code_preview_left.xview(*args)
        self._code_preview_text.xview(*args)

    def _on_code_mousewheel(self, event):
        """鼠标滚轮 → 同步滚动两个 Text 控件"""
        if event.num == 4:
            delta = -1
        elif event.num == 5:
            delta = 1
        else:
            delta = -1 if event.delta > 0 else 1
        self._code_preview_left.yview_scroll(delta, "units")
        self._code_preview_text.yview_scroll(delta, "units")
        return "break"

    @staticmethod
    def _build_aligned_diff(left_texts: list, right_texts: list
                            ) -> tuple:
        """按 diff 块对齐，返回 (left_aligned, right_aligned, tags)

        tags = [(widget_id, lineno, tag), ...]
        widget_id: 0=left, 1=right
        tag: "removed" or "added"
        """
        matcher = difflib.SequenceMatcher(None, left_texts, right_texts)

        new_left = []
        new_right = []
        tags = []  # (widget_id, lineno, tag)
        left_lineno = 1
        right_lineno = 1

        for op, i1, i2, j1, j2 in matcher.get_opcodes():
            if op == "equal":
                for k in range(i1, i2):
                    new_left.append(left_texts[k])
                    new_right.append(right_texts[k])
                    left_lineno += 1
                    right_lineno += 1
            elif op == "replace":
                ll = left_texts[i1:i2]
                rl = right_texts[j1:j2]
                mx = max(len(ll), len(rl))
                for k in range(mx):
                    ln = left_lineno + k
                    rn = right_lineno + k
                    if k < len(ll):
                        new_left.append(ll[k])
                        tags.append((0, ln, "removed"))
                    else:
                        new_left.append("\n")
                    if k < len(rl):
                        new_right.append(rl[k])
                        tags.append((1, rn, "added"))
                    else:
                        new_right.append("\n")
                left_lineno += mx
                right_lineno += mx
            elif op == "delete":
                for k in range(i1, i2):
                    ln = left_lineno
                    new_left.append(left_texts[k])
                    tags.append((0, ln, "removed"))
                    new_right.append("\n")
                    left_lineno += 1
                    right_lineno += 1
            elif op == "insert":
                for k in range(j1, j2):
                    rn = right_lineno
                    new_left.append("\n")
                    new_right.append(right_texts[k])
                    tags.append((1, rn, "added"))
                    left_lineno += 1
                    right_lineno += 1

        return "".join(new_left), "".join(new_right), tags

    # ================================================================
    # 代码预览
    # ================================================================
    def _update_code_preview(self, text: str, file_path: str = None):
        """更新代码预览 (右侧可编辑), 左侧显示 .bak 原内容"""
        self._code_preview_current_path = file_path

        # 左侧显示 .bak 原内容（如果存在）
        bak_path = file_path + ".bak" if file_path and os.path.isfile(file_path + ".bak") else None
        self._code_preview_left.config(state=tk.NORMAL)
        self._code_preview_left.delete(1.0, tk.END)
        if bak_path:
            with open(bak_path, "r", encoding="utf-8", errors="replace") as f:
                self._code_preview_left.insert(1.0, f.read())
        else:
            self._code_preview_left.insert(1.0, "(无 .bak 文件)")
        self._code_preview_left.config(state=tk.DISABLED)

        # 右侧显示生成内容（可编辑）
        self._code_preview_text.config(state=tk.NORMAL)
        self._code_preview_text.delete(1.0, tk.END)
        self._code_preview_text.insert(1.0, text)

    # ================================================================
    # 管道代码预览
    # ================================================================
    def _update_pipe_preview(self, text: str):
        """更新管道预览 (右侧可编辑)"""
        self._pipe_preview_text.config(state=tk.NORMAL)
        self._pipe_preview_text.delete(1.0, tk.END)
        self._pipe_preview_text.insert(1.0, text)

        # 管道预览无对应 .bak 文件
        self._pipe_preview_old.config(state=tk.NORMAL)
        self._pipe_preview_old.delete(1.0, tk.END)
        if text.strip():
            self._pipe_preview_old.insert(1.0, "(管道预览来自 JSON 配置，无对应 .bak 文件)")
        self._pipe_preview_old.config(state=tk.DISABLED)

    def _highlight_text_diff(self, left_widget: tk.Text, right_widget: tk.Text):
        """左右 Text 控件逐行对比高亮差异"""
        left_widget.tag_configure("removed", background="#ffe0e0")
        right_widget.tag_configure("added", background="#e0ffe0")

        left_text = left_widget.get(1.0, tk.END).splitlines(keepends=True)
        right_text = right_widget.get(1.0, tk.END).splitlines(keepends=True)

        matcher = difflib.SequenceMatcher(None, left_text, right_text)

        left_lineno = 1
        right_lineno = 1
        for op, i1, i2, j1, j2 in matcher.get_opcodes():
            if op == "equal":
                left_lineno += (i2 - i1)
                right_lineno += (j2 - j1)
            elif op == "replace":
                for k in range(i1, i2):
                    left_widget.tag_add("removed",
                        f"{left_lineno + k - i1}.0", f"{left_lineno + k - i1}.end")
                left_lineno += (i2 - i1)
                for k in range(j1, j2):
                    right_widget.tag_add("added",
                        f"{right_lineno + k - j1}.0", f"{right_lineno + k - j1}.end")
                right_lineno += (j2 - j1)
            elif op == "delete":
                for k in range(i1, i2):
                    left_widget.tag_add("removed",
                        f"{left_lineno + k - i1}.0", f"{left_lineno + k - i1}.end")
                left_lineno += (i2 - i1)
            elif op == "insert":
                for k in range(j1, j2):
                    right_widget.tag_add("added",
                        f"{right_lineno + k - j1}.0", f"{right_lineno + k - j1}.end")
                right_lineno += (j2 - j1)

    def _expand_type_refs_for_preview(self, fields: list) -> list:
        """将指针引用外部类型的字段展开为 inline struct，供管道预览展示"""
        result = []
        for f in fields:
            ftype = f.get("type", "")
            base_type = ftype.rstrip('*').strip()
            type_def = self._find_type_definition(base_type) if base_type and base_type != ftype else None
            if type_def:
                expanded = dict(f)
                expanded["type"] = "struct"
                expanded["comment"] = f"{ftype} {f.get('name', '')}: {f.get('comment', '')} (外部类型引用)"
                expanded["fields"] = list(type_def["fields"])
                result.append(expanded)
            else:
                result.append(f)
        return result

    def _show_pipe_preview(self, pipe):
        """管道预览: 显示该管道在两端模块中的生成代码 (不含完整模块 io.h)"""
        if not pipe:
            self._update_pipe_preview("")
            return
        from_name = pipe["from"]
        to_name = pipe["to"]
        fields = pipe.get("fields", [])
        out_style = pipe.get("out_link", {}).get("style", "array")
        out_size = pipe.get("out_link", {}).get("array_size", 4)
        in_style = pipe.get("in_link", {}).get("style", out_style)
        in_size = pipe.get("in_link", {}).get("array_size", out_size)

        from gen_io_h import _gen_params_fields, _link_style_to_c

        # 展开外部类型引用，让预览显示子字段
        preview_fields = self._expand_type_refs_for_preview(fields)

        lines = []
        lines.append("// ================================================================")
        lines.append(f"//  {from_name} → {to_name}")
        lines.append(f"//  {pipe.get('comment', '')}")
        lines.append(f"//  callback_type: {pipe.get('callback_type', 'pull')}")
        lines.append(f"//  LINK:  OUT={out_style}({out_size})   IN={in_style}({in_size})")
        lines.append("// ================================================================")
        lines.append("")

        # ---- OUTPUT 侧 (FROM_io.h) ----
        lines.append("/* ----- OUTPUT 侧  (From_io.h) ----- */")
        lines.append("")
        lines.append(f"/* {from_name} → {to_name}  输出参数 */")
        lines.append(f"typedef struct {{")
        lines.append(_gen_params_fields(preview_fields))
        lines.append(f"}} MODULE_OUTPUT_PARAMS({from_name}, {to_name});")
        lines.append("")
        out_link = _link_style_to_c(out_style, out_size)
        lines.append(f"typedef struct {{")
        lines.append(f"    uint8_t  status;           /* ST_NEW / ST_OUT */")
        lines.append(f"    uint8_t  max_count;        /* 最大数量 */")
        lines.append(f"    uint8_t  count;            /* 当前周期索引 */")
        lines.append(f"    uint8_t  res[1];")
        lines.append(f"    MODULE_OUTPUT_PARAMS({from_name}, {to_name}) {out_link};")
        lines.append(f"}} MODULE_OUTPUT_LINK({from_name}, {to_name});")

        lines.append("")
        lines.append("")
        # ---- INPUT 侧 (TO_io.h) ----
        lines.append("/* ----- INPUT 侧  (To_io.h) ----- */")
        lines.append("")
        lines.append(f"/* {from_name} → {to_name}  输入参数 (与 OUTPUT_PARAMS 布局兼容) */")
        lines.append(f"typedef struct {{")
        lines.append(_gen_params_fields(preview_fields))
        lines.append(f"}} MODULE_INPUT_PARAMS({from_name}, {to_name});")
        lines.append("")
        in_link = _link_style_to_c(in_style, in_size)
        lines.append(f"typedef struct {{")
        lines.append(f"    uint8_t  status;           /* ST_NEW / ST_OUT */")
        lines.append(f"    uint8_t  max_count;        /* 最大数量 */")
        lines.append(f"    uint8_t  count;            /* 当前周期索引 */")
        lines.append(f"    uint8_t  res[1];")
        lines.append(f"    MODULE_INPUT_PARAMS({from_name}, {to_name}) {in_link};")
        lines.append(f"}} MODULE_INPUT_LINK({from_name}, {to_name});")

        self._update_pipe_preview("\n".join(lines))

    # ================================================================
    # 管道预览 — 确认更新 JSON
    # ================================================================
    def _parse_fields_from_text(self, text: str) -> list:
        """从 C typedef struct 文本提取 fields: [{name, type, comment, ?fields}]

        支持: 简单字段, 数组, 指针, 嵌套 struct { ... } name;
        """
        import re
        fields = []

        # 找到 "typedef struct {" 后的第一个顶层 '{'
        body_start = text.find("typedef struct {")
        if body_start < 0:
            body_start = text.find("typedef struct\n{")
        if body_start < 0:
            return fields
        brace_start = text.find("{", body_start)
        if brace_start < 0:
            return fields

        # 括号匹配找到顶层 '}' (跳过嵌套 struct 内的 '}')
        depth = 1
        i = brace_start + 1
        while i < len(text) and depth > 0:
            if text[i] == '{':
                depth += 1
            elif text[i] == '}':
                depth -= 1
            i += 1
        body = text[brace_start+1:i-1]

        # 逐行解析
        for line in body.splitlines():
            line = line.strip()
            if not line or line.startswith("//") or line.startswith("/*"):
                continue

            # 嵌套 struct 开始: struct { /* comment */
            ns = re.match(r'^struct\s*\{\s*/\*\s*(.*?)\s*\*/\s*$', line)
            if ns:
                continue

            # 嵌套 struct 结束: } name; 或 } name; /* comment */
            ne = re.match(r'^\}\s*(\w+)\s*;\s*(?:/\*\s*(.*?)\s*\*/\s*)?$', line)
            if ne:
                continue

            # 普通字段: [type] name; 或 [type] name[N]; 或 [type]* name;
            # type 可能含 * 如 AppAdc_OutputParams_t*  或 uint16_t**
            m = re.match(
                r'^((?:\w+(?:\s+\w+)*(?:\s*\*+)?)|(?:\w+(?:\s*\*+)?))\s+'
                r'(\w+)'
                r'(?:\[(\d+)\])?\s*;\s*(?:/\*\s*(.*?)\s*\*/\s*)?$', line)
            if m:
                base_type = m.group(1).strip()
                fname = m.group(2)
                arr_size = m.group(3)
                comment = (m.group(4) or "").strip()
                if arr_size:
                    ftype = f"{base_type}[{arr_size}]"
                else:
                    ftype = base_type
                fields.append({"name": fname, "type": ftype, "comment": comment})
                continue

            # 未知格式 — 跳过

        return fields

    def _pipe_preview_confirm(self):
        """解析管道预览编辑区的文本 → 更新 JSON fields → 刷新字段树"""
        if not self._selected_pipe:
            messagebox.showinfo("提示", "请先选择一个管道")
            return
        text = self._pipe_preview_text.get(1.0, tk.END)
        parsed = self._parse_fields_from_text(text)
        if not parsed:
            if not messagebox.askyesno("确认", "未解析到字段定义，是否仍更新？"):
                return
        self._selected_pipe["fields"] = parsed
        self._refresh_field_table(self._selected_pipe)
        self.status(f"JSON 已更新: {len(parsed)} 个字段")

    def _pipe_preview_save_as(self):
        """另存管道预览文本"""
        text = self._pipe_preview_text.get(1.0, tk.END).strip()
        if not text:
            messagebox.showinfo("提示", "没有内容可保存")
            return
        path = filedialog.asksaveasfilename(
            defaultextension=".txt",
            filetypes=[("C 源码", "*.c *.h"), ("文本", "*.txt"), ("所有", "*.*")],
            title="另存管道预览")
        if not path:
            return
        with open(path, "w", encoding="utf-8") as f:
            f.write(text)
        self.status(f"已保存: {path}")

    # ================================================================
    # 代码预览 — 更新 / 另存
    # ================================================================
    def _code_preview_update(self):
        """更新 AI 生成段: 先时间戳备份, 再写入"""
        text = self._code_preview_text.get(1.0, tk.END)
        if not text.strip():
            messagebox.showinfo("提示", "没有内容可写入")
            return
        path = self._code_preview_current_path
        if not path:
            messagebox.showinfo("提示", "请先选择一个模块 (没有关联的源文件路径)")
            return
        if not messagebox.askyesno("确认更新",
                                   f"更新 AI 生成段?\n{path}\n\n将自动时间戳备份"):
            return
        try:
            if os.path.isfile(path):
                with open(path, "r", encoding="utf-8", errors="replace") as f:
                    original = f.read()
                old_ai_start, old_ai_end = find_ai_block(original)
                if old_ai_start >= 0:
                    # 旧文件有 AI 标记 → 替换 AI 段，保留头部/尾部
                    ai_start, ai_end = find_ai_block(text)
                    if ai_start >= 0:
                        new_ai_part = text[ai_start:ai_end]
                        before_old_ai = original[:old_ai_start]
                        if not any(x in before_old_ai for x in ['#ifndef', '#include']):
                            content = before_old_ai + text  # 迁移: 旧格式
                        else:
                            content = before_old_ai + new_ai_part + original[old_ai_end:]  # 稳态
                    else:
                        content = text  # 预览无 AI 标记, 直接写
                else:
                    # 旧文件无 AI 标记 → 写入预览内容 (生成器已产生正确输出)
                    content = text
            else:
                content = text  # 新文件

            import time
            bak_path = ""
            if os.path.isfile(path):
                bak_path = f"{path}.bak_{time.strftime('%Y%m%d_%H%M%S')}"
                shutil.copy2(path, bak_path)
            os.makedirs(os.path.dirname(path), exist_ok=True)
            with open(path, "w", encoding="utf-8") as f:
                f.write(content)
            bak_info = f"  (备份: {os.path.basename(bak_path)})" if bak_path else ""
            self.status(f"已更新: {path}{bak_info}")
        except Exception as e:
            messagebox.showerror("写入失败", str(e))
            import traceback
            traceback.print_exc()

    def _code_preview_save_as(self):
        """另存代码预览文件"""
        text = self._code_preview_text.get(1.0, tk.END).strip()
        if not text:
            messagebox.showinfo("提示", "没有内容可保存")
            return
        path = filedialog.asksaveasfilename(
            defaultextension=".c",
            filetypes=[("C 源码", "*.c *.h"), ("所有", "*.*")],
            title="另存代码预览")
        if not path:
            return
        with open(path, "w", encoding="utf-8") as f:
            f.write(text)
        self.status(f"已保存: {path}")

    def _code_preview_toggle_diff(self):
        """切换显示/隐藏 .bak 差异对比面板"""
        panes = self._code_preview_pw.panes()
        left_in = self._code_preview_left_frame in panes
        if left_in:
            self._code_preview_pw.forget(self._code_preview_left_frame)
        else:
            self._code_preview_pw.insert(0, self._code_preview_left_frame, weight=1)
    def _show_data_switcher_preview(self):
        """生成 data_switcher 代码预览"""
        config = self._collect_config()
        project = config.get("project", {})
        paths = project.get("paths", {})
        output_root = self.entry_output_root.get().strip()
        core_dir = paths.get("core_dir", "core")
        from gen_switcher import generate_switcher
        content = generate_switcher(
            config.get("modules", []),
            config.get("pipes", []),
            config.get("slot_order", []),
            project,
            config.get("slot_chains", [])
        )
        sw_path = os.path.abspath(os.path.join(output_root, core_dir, "data_switcher.c")) if output_root else ""
        self._code_preview_module_label.config(text="data_switcher")
        self._update_code_preview(content, sw_path)


    def _code_preview_regenerate(self):
        """重新生成当前代码预览内容，不写磁盘"""
        module_name = self._code_preview_module_label.cget("text")
        if not module_name or module_name == "(未选择模块)":
            messagebox.showinfo("提示", "请先选择一个模块")
            return
        file_type = self._code_preview_type.get()
        # data_switcher 特殊处理
        if module_name == "data_switcher":
            self._show_data_switcher_preview()
            self.status("重新生成: data_switcher")
            return
        mod = next((m for m in self.modules if m["name"] == module_name), None)
        if not mod:
            messagebox.showinfo("提示", f"模块 {module_name} 未找到")
            return
        self.status(f"重新生成: {mod['name']} ({file_type})")
        self._show_module_preview(mod, file_type)

    def _check_append(self, text: str, tag: str = None):
        """向检查结果末尾追加一行"""
        self.check_text.config(state=tk.NORMAL)
        self.check_text.insert(tk.END, text + "\n", tag)
        self.check_text.see(tk.END)
        self.check_text.config(state=tk.DISABLED)

    def _clear_check_results(self):
        self.check_text.config(state=tk.NORMAL)
        self.check_text.delete(1.0, tk.END)
        self.check_text.config(state=tk.DISABLED)
        self.lbl_check_summary.config(text="(已清空)")

    def _check_tag(self, tag: str = None):
        """根据 tag 名给 check_text 配置颜色"""
        self.check_text.tag_configure("pass", foreground="green")
        self.check_text.tag_configure("fail", foreground="red")
        self.check_text.tag_configure("skip", foreground="gray")
        self.check_text.tag_configure("info", foreground="blue")
        self.check_text.tag_configure("bold", font=("Consolas", 10, "bold"))

    def cmd_force_refresh(self):
        """强制刷新所有预览内容（重新生成树、管道预览、代码预览）"""
        if not self.config:
            return
        # 记住当前选择
        sel = self.tree.selection()
        # 重建模块树
        self._rebuild_tree()
        # 重新选择并触发更新
        if sel:
            try:
                self.tree.selection_set(sel[0])
                self._on_tree_select(None)
            except tk.TclError:
                pass
        self.status("强制刷新完成")

    def cmd_check(self):
        """运行全部检查"""
        if not self.config:
            messagebox.showinfo("提示", "请先打开或创建配置文件")
            return

        self._clear_check_results()
        self._check_tag()
        self.notebook.select(2)  # 切换到检查结果 tab
        self.status("正在检查...")
        self.root.update()

        results = []  # [(name, status, detail)]
        self._check_append("=" * 60, "bold")
        self._check_append("  v2.3 LINK+PARAMS 合规检查", "bold")
        self._check_append("=" * 60, "bold")
        self._check_append("")

        # ---- JSON 配置检查 ----
        self._check_append("--- JSON 配置检查 ---", "bold")

        r = self._check_schema()
        results.append(r)
        self._check_append(r["detail"])

        r = self._check_module_existence()
        results.append(r)
        self._check_append(r["detail"])

        r = self._check_slot_order()
        results.append(r)
        self._check_append(r["detail"])

        r = self._check_pipe_id_unique()
        results.append(r)
        self._check_append(r["detail"])

        r = self._check_fields_comment()
        results.append(r)
        self._check_append(r["detail"])

        r = self._check_link_symmetry()
        results.append(r)
        self._check_append(r["detail"])

        # ---- 项目文件检查 ----
        self._check_append("")
        self._check_append("--- 生成文件检查 ---", "bold")

        r = self._check_io_files()
        results.append(r)
        self._check_append(r["detail"])

        r = self._check_module_source_files()
        results.append(r)
        self._check_append(r["detail"])

        r = self._check_io_constants()
        results.append(r)
        self._check_append(r["detail"])

        # ---- 汇总 ----
        self._check_append("")
        self._check_append("-" * 60)
        passed = sum(1 for r in results if r["status"] == "PASS")
        failed = sum(1 for r in results if r["status"] == "FAIL")
        skipped = sum(1 for r in results if r["status"] == "SKIP")

        if failed == 0:
            summary = f"通过: {passed}  跳过: {skipped}  失败: {failed} — 检查通过"
            self._check_append(f"  ✓ {summary}", "pass")
            self.lbl_check_summary.config(text=f"✓ {passed}/{len(results)} 通过，{failed} 失败，{skipped} 跳过")
        else:
            summary = f"通过: {passed}  跳过: {skipped}  失败: {failed} — 需要修复"
            self._check_append(f"  ✗ {summary}", "fail")
            self.lbl_check_summary.config(text=f"✗ {failed} 项失败 — 需要修复")

        self._check_append("=" * 60, "bold")
        self.status("检查完成")

    # ---- 单项检查 ----
    def _check_schema(self):
        """检查 JSON 配置的完整性"""
        issues = []
        data = self._collect_config()

        # 检查必需字段
        if not data.get("project"):
            issues.append("缺少 project 段")
        if not data.get("modules"):
            issues.append("modules 为空")
        if not data.get("pipes"):
            issues.append("pipes 为空（无管道连接）")
        if not data.get("slot_order"):
            issues.append("slot_order 为空")

        # 检查 modules 完整性
        for i, m in enumerate(data.get("modules", [])):
            if not m.get("id"):
                issues.append(f"modules[{i}].id 为空")
            if not m.get("name"):
                issues.append(f"modules[{i}].name 为空")
            if not m.get("layer"):
                issues.append(f"modules[{i}].layer 为空")
            if not m.get("comment", "").strip():
                issues.append(f"modules[{i}].comment 不可为空（{m.get('name', '?')}）")

        if issues:
            detail = "[FAIL] JSON 配置完整性: " + "; ".join(issues)
            return {"status": "FAIL", "detail": detail}
        return {"status": "PASS", "detail": "[PASS] JSON 配置完整性 — 结构完整"}

    def _check_module_existence(self):
        """检查所有 pipe 引用的模块是否存在"""
        mod_names = {m["name"] for m in self.modules}
        issues = []
        for p in self.pipes:
            if p["from"] not in mod_names:
                issues.append(f"pipe '{p['id']}': from='{p['from']}' 不在 modules 中")
            if p["to"] not in mod_names:
                issues.append(f"pipe '{p['id']}': to='{p['to']}' 不在 modules 中")

        if issues:
            detail = "[FAIL] 模块引用: " + "; ".join(issues)
            return {"status": "FAIL", "detail": detail}
        return {"status": "PASS", "detail": f"[PASS] 模块引用 — {len(self.pipes)} 条管道全部有效"}

    def _check_slot_order(self):
        """检查 slot_order 是否覆盖所有模块"""
        mod_names = {m["name"] for m in self.modules}
        slot_set = set(self.slot_order)
        missing = mod_names - slot_set
        extra = slot_set - mod_names
        issues = []
        if missing:
            issues.append(f"模块未在 slot_order 中: {', '.join(sorted(missing))}")
        if extra:
            issues.append(f"slot_order 中有不存在的模块: {', '.join(sorted(extra))}")

        if issues:
            detail = "[FAIL] slot_order: " + "; ".join(issues)
            return {"status": "FAIL", "detail": detail}
        return {"status": "PASS", "detail": f"[PASS] slot_order — {len(self.slot_order)} 个模块全部到位"}

    def _check_pipe_id_unique(self):
        """检查 pipe id 是否唯一"""
        ids = [p["id"] for p in self.pipes]
        dupes = set(i for i in ids if ids.count(i) > 1)
        if dupes:
            detail = f"[FAIL] pipe id 重复: {', '.join(dupes)}"
            return {"status": "FAIL", "detail": detail}
        return {"status": "PASS", "detail": f"[PASS] pipe id 唯一 — {len(ids)} 条无重复"}

    def _check_fields_comment(self):
        """检查 fields 中的 comment 是否都填了"""
        empty = []
        for p in self.pipes:
            for i, f in enumerate(p.get("fields", [])):
                if not f.get("comment", "").strip():
                    empty.append(f"pipe '{p['id']}' field[{i}] '{f.get('name', '?')}'")

        if empty:
            detail = "[FAIL] 字段注释缺失: " + "; ".join(empty[:10])
            if len(empty) > 10:
                detail += f" ...（共 {len(empty)} 处）"
            return {"status": "FAIL", "detail": detail}
        return {"status": "PASS", "detail": f"[PASS] 字段注释 — 全部填写"}

    def _check_link_symmetry(self):
        """检查 out_link 和 in_link 的 style 是否对称"""
        issues = []
        for p in self.pipes:
            out_style = p.get("out_link", {}).get("style")
            in_style = p.get("in_link", {}).get("style")
            if out_style != in_style:
                issues.append(f"pipe '{p['id']}': out={out_style} ≠ in={in_style}")
        if issues:
            detail = "[FAIL] LINK 不对称: " + "; ".join(issues)
            return {"status": "FAIL", "detail": detail}
        return {"status": "PASS", "detail": f"[PASS] LINK 对称 — {len(self.pipes)} 条管道 OUT=IN"}

    def _check_io_files(self):
        """检查生成的 io.h 文件是否存在"""
        output_root = self.entry_output_root.get().strip()
        if not output_root:
            return {"status": "SKIP", "detail": "[SKIP] io.h 检查: 未设置输出根目录"}
        if not os.path.isdir(output_root):
            return {"status": "SKIP", "detail": f"[SKIP] io.h 检查: 目录不存在 ({output_root})"}

        io_dir = self.entry_io_dir.get().strip() or "include"
        io_path = os.path.join(output_root, io_dir)
        if not os.path.isdir(io_path):
            return {"status": "SKIP", "detail": f"[SKIP] io.h 检查: {io_dir}/ 目录不存在"}

        missing = []
        for m in self.modules:
            fname = f"{_pascal_to_snake(m['name'])}_io.h"
            if not os.path.isfile(os.path.join(io_path, fname)):
                missing.append(fname)

        if missing:
            detail = f"[FAIL] 缺少 {len(missing)} 个 io.h: {', '.join(missing)}"
            return {"status": "FAIL", "detail": detail}
        return {"status": "PASS", "detail": f"[PASS] io.h 文件 — {len(self.modules)} 个全部存在 ({io_path})"}

    def _check_module_source_files(self):
        """检查模块源文件是否存在"""
        output_root = self.entry_output_root.get().strip()
        if not output_root:
            return {"status": "SKIP", "detail": "[SKIP] 源文件检查: 未设置输出根目录"}
        if not os.path.isdir(output_root):
            return {"status": "SKIP", "detail": f"[SKIP] 源文件检查: 目录不存在 ({output_root})"}

        missing = []
        for m in self.modules:
            src = m.get("source_file", "")
            if not src:
                continue
            full_path = os.path.join(output_root, src)
            if not os.path.isfile(full_path):
                # 也尝试直接找 .c 文件
                alt_path = os.path.join(output_root, m["layer"], f"{_pascal_to_snake(m['name'])}.c")
                if not os.path.isfile(alt_path):
                    missing.append(src)

        if missing:
            detail = f"[FAIL] 缺少 {len(missing)} 个源文件: {', '.join(missing[:8])}"
            if len(missing) > 8:
                detail += f" ...（共 {len(missing)} 个）"
            return {"status": "FAIL", "detail": detail}
        return {"status": "PASS", "detail": f"[PASS] 源文件 — {len(self.modules)} 个模块文件存在"}

    def _check_io_constants(self):
        """检查 io.h 中是否包含 #define 常量（应迁移至 .c）"""
        output_root = self.entry_output_root.get().strip()
        if not output_root:
            return {"status": "SKIP", "detail": "[SKIP] io.h 常量检查: 未设置输出根目录"}
        if not os.path.isdir(output_root):
            return {"status": "SKIP", "detail": "[SKIP] io.h 常量检查: 目录不存在"}

        io_dir = self.entry_io_dir.get().strip() or "include"
        io_path = os.path.join(output_root, io_dir)
        if not os.path.isdir(io_path):
            return {"status": "SKIP", "detail": "[SKIP] io.h 常量检查: include/ 目录不存在"}

        import re
        issues = []
        for fname in sorted(os.listdir(io_path)):
            if not fname.endswith("_io.h"):
                continue
            fpath = os.path.join(io_path, fname)
            try:
                with open(fpath, "r", encoding="utf-8", errors="replace") as f:
                    for i, line in enumerate(f, 1):
                        if re.match(r'^\s*#\s*define\s+\w+\s+\S', line):
                            # 跳过 include guard
                            if re.match(r'^\s*#\s*define\s+\w+_IO_H\s', line):
                                continue
                            issues.append(f"  {fname}:{i}  {line.strip()}")
            except Exception:
                issues.append(f"  {fname}: (读取出错)")

        if issues:
            detail = "[FAIL] io.h 中存在 #define 常量，建议迁移至 .c:\n" + "\n".join(issues[:15])
            if len(issues) > 15:
                detail += f"\n  ...（共 {len(issues)} 处）"
            return {"status": "FAIL", "detail": detail}
        return {"status": "PASS", "detail": "[PASS] io.h 常量检查 — 未发现 #define 常量"}

    # ================================================================
    # 生成预览 (内存中生成，不写磁盘)
    # ================================================================
    def _preview_generation(self, mode: str) -> list:
        """生成所有文件内容到内存，返回 [{"path","new_content","original_content"}, ...]"""
        config = self._collect_config()
        project = config.get("project", {})
        paths = project.get("paths", {})
        modules = config.get("modules", [])
        pipes = config.get("pipes", [])
        slot_order = config.get("slot_order", [m["name"] for m in modules])
        slot_chains = config.get("slot_chains", [])

        io_dir    = paths.get("io_dir", "include")
        core_dir  = paths.get("core_dir", "core")
        app_dir   = paths.get("app_dir", "app")
        base_dir  = paths.get("base_class_dir", "base_class")
        proto_dir = paths.get("proto_dir", "proto")
        output_root = self.entry_output_root.get().strip()

        def _mod_output_dir(module: dict) -> str:
            layer = module.get("layer", "app")
            if layer == "app":
                return os.path.join(output_root, app_dir)
            elif layer == "base_class":
                return os.path.join(output_root, base_dir)
            elif layer == "proto":
                return os.path.join(output_root, proto_dir)
            else:
                return os.path.join(output_root, layer)

        files = []

        # 1. io.h (AI 标记内嵌在 generate_io_h 输出中, 仅范式段可被替换)
        if mode in ("all", "io"):
            io_out = os.path.join(output_root, io_dir)
            for mod in modules:
                raw_content = generate_io_h(mod, pipes, project)
                fname = f"{_pascal_to_snake(mod['name'])}_io.h"
                fpath = os.path.join(io_out, fname)
                original = ""
                if os.path.isfile(fpath):
                    with open(fpath, "r", encoding="utf-8", errors="replace") as f:
                        original = f.read()
                    # 从新内容中提取 AI 段
                    ai_start, ai_end = find_ai_block(raw_content)
                    if ai_start >= 0:
                        new_ai_part = raw_content[ai_start:ai_end]
                        # 检查旧文件中 AI 标记前的区域是否包含标准头部
                        old_ai_start, old_ai_end = find_ai_block(original)
                        if old_ai_start >= 0:
                            before_old_ai = original[:old_ai_start]
                            # 迁移判定: 旧 AI 标记前没有 #ifndef/#include → 头部在 AI 块内部
                            if not any(x in before_old_ai for x in ['#ifndef', '#include']):
                                # 迁移模式: 用户内容(标记前) + 新完整内容(头部+AI段+尾部)
                                content = before_old_ai + raw_content
                            else:
                                # 稳态: 保留旧头部/尾部, 只换 AI 段
                                content = before_old_ai + new_ai_part + original[old_ai_end:]
                        else:
                            # 旧文件无 AI 标记 → 插入到头部
                            content = raw_content
                    else:
                        # 新内容没有 AI 标记（异常）, 回退到完整替换
                        content = raw_content
                else:
                    # 新文件: 直接使用完整内容（自带 AI 标记包围范式段）
                    content = raw_content
                files.append({"path": fpath, "new_content": content, "original_content": original})

        # 2. data_switcher.c (包裹 AI 标记, 统一处理)
        if mode in ("all", "switcher"):
            core_out = os.path.join(output_root, core_dir)
            raw_content = generate_switcher(modules, pipes, slot_order, project, slot_chains)
            content = wrap_in_ai_block(raw_content)
            fpath = os.path.join(core_out, "data_switcher.c")
            original = ""
            if os.path.isfile(fpath):
                with open(fpath, "r", encoding="utf-8", errors="replace") as f:
                    original = f.read()
            files.append({"path": fpath, "new_content": content, "original_content": original})

        # 3. 模块 .c (仅新模块, 不覆盖已有源文件)
        if mode in ("all", "modules"):
            for mod in modules:
                source_file = mod.get("source_file", "")
                if source_file:
                    c_path = os.path.abspath(os.path.join(output_root, source_file))
                else:
                    mod_out = _mod_output_dir(mod)
                    c_path = os.path.join(mod_out, f"{_pascal_to_snake(mod['name'])}.c")

                # 已有 .c 文件: 使用重构模式插入 AI 块到头部
                if os.path.isfile(c_path):
                    with open(c_path, "r", encoding="utf-8", errors="replace") as f:
                        orig = f.read()
                    content_c = generate_module_c_refactored(mod, pipes, project, c_path)
                    files.append({"path": c_path, "new_content": content_c, "original_content": orig})
                    continue

                content_c = generate_module_c(mod, pipes, project, c_path)
                files.append({"path": c_path, "new_content": content_c, "original_content": ""})

        return files

    # ================================================================
    # 生成
    # ================================================================
    def cmd_generate(self, mode: str):
        """调用生成器 — 预览→确认→备份→写入"""
        if not self.config:
            messagebox.showinfo("提示", "请先打开或创建配置文件")
            return

        output_root = self.entry_output_root.get().strip()
        if not output_root:
            messagebox.showerror("错误", "请填写输出根目录")
            return

        self.status("正在预生成...")
        self.root.update()

        try:
            files = self._preview_generation(mode)
        except Exception as e:
            self.status("预生成失败")
            messagebox.showerror("预生成失败", str(e))
            return

        if not files:
            messagebox.showinfo("提示", "没有文件需要生成")
            return

        # 弹出 DiffDialog 让用户确认
        dlg = DiffDialog(self.root, files)
        if not dlg.result:
            self.status("用户取消生成")
            return

        # 备份 + 写入
        count = 0
        for f in dlg.result:
            fpath = f["path"]
            if os.path.isfile(fpath):
                try:
                    shutil.copy2(fpath, fpath + ".bak")
                except Exception as e:
                    self.status(f"备份失败: {os.path.basename(fpath)}")
            os.makedirs(os.path.dirname(fpath), exist_ok=True)
            with open(fpath, "w", encoding="utf-8") as fp:
                fp.write(f["new_content"])
            count += 1

        self.status(f"生成完成: {count} 个文件")
        messagebox.showinfo("生成结果",
            f"成功生成 {count} 个文件到:\n{os.path.abspath(output_root)}")

    # ================================================================
    # 对话框
    # ================================================================
    def cmd_about(self):
        messagebox.showinfo("关于", "v2.3 LINK+PARAMS 配置编辑器\n\n"
                                   "根据 JSON 配置自动生成:\n"
                                   "  - include/*_io.h\n"
                                   "  - core/data_switcher.c\n"
                                   "  - {layer}/*.c / .h")


class DiffDialog:
    """生成预览对话框 — 确认更新 AI 生成段"""

    def __init__(self, parent, files_to_generate):
        self.result = []  # 确认的文件列表，_execute 时填充
        self.files = files_to_generate

        dlg = tk.Toplevel(parent)
        dlg.title("生成预览 — 确认更新 AI 生成段")
        dlg.geometry("1100x700")
        dlg.transient(parent)
        dlg.grab_set()

        # === 顶部: 文件列表 ===
        top_frame = ttk.LabelFrame(dlg, text="文件列表", padding=4)
        top_frame.pack(fill=tk.X, padx=6, pady=(6, 2))

        columns = ("sel", "file", "status")
        self.file_tree = ttk.Treeview(top_frame, columns=columns, displaycolumns=columns, height=6, selectmode="browse")
        self.file_tree.pack(fill=tk.X)
        self.file_tree.heading("sel", text="☐")
        self.file_tree.heading("file", text="文件路径")
        self.file_tree.heading("status", text="状态")
        self.file_tree.column("sel", width=30, anchor=tk.CENTER)
        self.file_tree.column("file", width=450)
        self.file_tree.column("status", width=80, anchor=tk.CENTER)
        self.file_tree.bind("<ButtonRelease-1>", self._on_file_click_select)
        self.file_tree.bind("<<TreeviewSelect>>", self._on_file_select_diff)

        self._file_vars = {}  # {iid: bool}
        for f in files_to_generate:
            path = f["path"]
            exists = os.path.isfile(path)
            status = "修改" if exists else "新增"
            iid = self.file_tree.insert("", tk.END, values=("☑", path, status))
            self._file_vars[iid] = True

        # === 中部: 差异对比 ===
        diff_frame = ttk.LabelFrame(dlg, text="差异对比", padding=4)
        diff_frame.pack(fill=tk.BOTH, expand=True, padx=6, pady=(2, 6))

        # 左: 原文件
        left_frame = ttk.Frame(diff_frame)
        left_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        ttk.Label(left_frame, text="原文件", font=("", 9, "bold")).pack(anchor=tk.W)
        self.text_old = tk.Text(left_frame, wrap=tk.NONE, font=("Consolas", 9), bg="#fafafa")
        self.text_old.pack(fill=tk.BOTH, expand=True)

        # 右: 新生成
        right_frame = ttk.Frame(diff_frame)
        right_frame.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True, padx=(4, 0))
        ttk.Label(right_frame, text="新生成", font=("", 9, "bold")).pack(anchor=tk.W)
        self.text_new = tk.Text(right_frame, wrap=tk.NONE, font=("Consolas", 9), bg="#fafafa")
        self.text_new.pack(fill=tk.BOTH, expand=True)

        # 同步滚动条
        vsb = ttk.Scrollbar(diff_frame, orient=tk.VERTICAL)
        vsb.pack(side=tk.RIGHT, fill=tk.Y)
        vsb.config(command=self._sync_scroll)
        self.text_old.configure(yscrollcommand=lambda *a: vsb.set(*a))
        self.text_new.configure(yscrollcommand=lambda *a: vsb.set(*a))

        hsb = ttk.Scrollbar(dlg, orient=tk.HORIZONTAL)
        hsb.pack(fill=tk.X, padx=6)
        hsb.config(command=self._sync_hscroll)
        self.text_old.configure(xscrollcommand=lambda *a: hsb.set(*a))
        self.text_new.configure(xscrollcommand=lambda *a: hsb.set(*a))

        # === 底部: 按钮 ===
        btn_frame = ttk.Frame(dlg)
        btn_frame.pack(fill=tk.X, padx=6, pady=(0, 6))

        ttk.Button(btn_frame, text="全选", command=self._select_all).pack(side=tk.LEFT, padx=2)
        ttk.Button(btn_frame, text="取消全选", command=self._deselect_all).pack(side=tk.LEFT, padx=2)
        ttk.Label(btn_frame, text="").pack(side=tk.LEFT, fill=tk.X, expand=True)

        self.btn_execute = ttk.Button(btn_frame, text=f"更新 AI 段 ({len(files_to_generate)} 个)", command=self._execute)
        self.btn_execute.pack(side=tk.RIGHT, padx=2)
        ttk.Button(btn_frame, text="取消", command=dlg.destroy).pack(side=tk.RIGHT, padx=2)

        # 显示第一个文件的 diff
        children = self.file_tree.get_children()
        if children:
            self.file_tree.selection_set(children[0])
            self._show_diff(0)

        self.dlg = dlg
        dlg.wait_window()

    def _on_file_click_select(self, event):
        iid = self.file_tree.identify_row(event.y)
        if not iid or iid not in self._file_vars:
            return
        self._file_vars[iid] = not self._file_vars[iid]
        self.file_tree.set(iid, "sel", "☑" if self._file_vars[iid] else "☐")
        # 更新按钮计数
        checked = sum(1 for v in self._file_vars.values() if v)
        self.btn_execute.config(text=f"更新 AI 段 ({checked} 个)")

    def _on_file_select_diff(self, event):
        sel = self.file_tree.selection()
        if not sel:
            return
        idx = self.file_tree.index(sel[0])
        self._show_diff(idx)

    def _show_diff(self, idx):
        f = self.files[idx]
        old_content = f.get('original_content', '')
        new_content = f['new_content']

        self.text_old.delete(1.0, tk.END)
        self.text_new.delete(1.0, tk.END)

        if not old_content:
            old_content = '(新文件，无原始内容)'

        old_lines = old_content.splitlines(keepends=True)
        new_lines = new_content.splitlines(keepends=True)

        # 复用 ConfigEditor 的对齐方法
        aligned_old, aligned_new, tags = ConfigEditor._build_aligned_diff(old_lines, new_lines)

        self.text_old.insert(1.0, aligned_old)
        self.text_new.insert(1.0, aligned_new)

        # 应用标签
        self.text_old.tag_configure('removed', background='#ffe0e0')
        self.text_new.tag_configure('added', background='#e0ffe0')
        for widget_id, lineno, tag in tags:
            w = self.text_old if widget_id == 0 else self.text_new
            w.tag_add(tag, f'{lineno}.0', f'{lineno}.end')


    def _sync_scroll(self, *args):
        self.text_old.yview(*args)
        self.text_new.yview(*args)

    def _sync_hscroll(self, *args):
        self.text_old.xview(*args)
        self.text_new.xview(*args)

    def _select_all(self):
        for iid in self._file_vars:
            self._file_vars[iid] = True
            self.file_tree.set(iid, "sel", "☑")
        checked = len(self._file_vars)
        self.btn_execute.config(text=f"更新 AI 段 ({checked} 个)")

    def _deselect_all(self):
        for iid in self._file_vars:
            self._file_vars[iid] = False
            self.file_tree.set(iid, "sel", "☐")
        self.btn_execute.config(text="更新 AI 段 (0 个)")

    def _execute(self):
        confirmed = []
        for i, iid in enumerate(self.file_tree.get_children()):
            if self._file_vars.get(iid, True):
                confirmed.append(self.files[i])
        self.result = confirmed
        self.dlg.destroy()


class ModuleDialog:
    """新增模块对话框"""

    def __init__(self, parent, existing_modules):
        self.result = None
        dlg = tk.Toplevel(parent)
        dlg.title("新增模块")
        dlg.geometry("360x200")
        dlg.resizable(False, False)
        dlg.transient(parent)
        dlg.grab_set()

        ttk.Label(dlg, text="模块名 (PascalCase):").pack(pady=(12, 2), anchor=tk.W, padx=12)
        entry_name = ttk.Entry(dlg, width=30)
        entry_name.pack(padx=12, fill=tk.X)
        entry_name.focus_set()

        ttk.Label(dlg, text="层:").pack(pady=(6, 2), anchor=tk.W, padx=12)
        cb_layer = ttk.Combobox(dlg, values=["app", "base_class", "proto", "core"], state="readonly")
        cb_layer.pack(padx=12, fill=tk.X)
        cb_layer.set("app")

        ttk.Label(dlg, text="注释:").pack(pady=(6, 2), anchor=tk.W, padx=12)
        entry_comment = ttk.Entry(dlg, width=30)
        entry_comment.pack(padx=12, fill=tk.X)

        btn_frame = ttk.Frame(dlg)
        btn_frame.pack(pady=(12, 8))
        ttk.Button(btn_frame, text="确定", command=lambda: self._ok(dlg, entry_name, cb_layer, entry_comment)).pack(
            side=tk.LEFT, padx=6)
        ttk.Button(btn_frame, text="取消", command=dlg.destroy).pack(side=tk.LEFT, padx=6)

        dlg.wait_window()

    def _ok(self, dlg, entry_name, cb_layer, entry_comment):
        name = entry_name.get().strip()
        if not name:
            messagebox.showerror("错误", "模块名不能为空", parent=dlg)
            return
        self.result = (name, cb_layer.get(), entry_comment.get().strip())
        dlg.destroy()


class PipeDialog:
    """新增管道对话框"""

    def __init__(self, parent, mod_names, existing_pipes):
        self.result = None
        dlg = tk.Toplevel(parent)
        dlg.title("新增管道")
        dlg.geometry("360x220")
        dlg.resizable(False, False)
        dlg.transient(parent)
        dlg.grab_set()

        ttk.Label(dlg, text="源模块 (Producer):").pack(pady=(12, 2), anchor=tk.W, padx=12)
        cb_from = ttk.Combobox(dlg, values=mod_names, state="readonly")
        cb_from.pack(padx=12, fill=tk.X)
        cb_from.set(mod_names[0])

        ttk.Label(dlg, text="目标模块 (Consumer):").pack(pady=(6, 2), anchor=tk.W, padx=12)
        cb_to = ttk.Combobox(dlg, values=mod_names, state="readonly")
        cb_to.pack(padx=12, fill=tk.X)
        if len(mod_names) > 1:
            cb_to.set(mod_names[1])

        ttk.Label(dlg, text="callback_type:").pack(pady=(6, 2), anchor=tk.W, padx=12)
        cb_cb = ttk.Combobox(dlg, values=["pull", "edge", "field_copy"], state="readonly")
        cb_cb.pack(padx=12, fill=tk.X)
        cb_cb.set("pull")

        btn_frame = ttk.Frame(dlg)
        btn_frame.pack(pady=(12, 8))
        ttk.Button(btn_frame, text="确定", command=lambda: self._ok(dlg, cb_from, cb_to, cb_cb)).pack(
            side=tk.LEFT, padx=6)
        ttk.Button(btn_frame, text="取消", command=dlg.destroy).pack(side=tk.LEFT, padx=6)

        dlg.wait_window()

    def _ok(self, dlg, cb_from, cb_to, cb_cb):
        frm = cb_from.get().strip()
        to = cb_to.get().strip()
        if not frm or not to:
            messagebox.showerror("错误", "请选择源模块和目标模块", parent=dlg)
            return
        if frm == to:
            messagebox.showerror("错误", "源模块和目标模块不能相同", parent=dlg)
            return
        self.result = (frm, to, cb_cb.get())
        dlg.destroy()


def main():
    root = tk.Tk()
    app = ConfigEditor(root)

    # 命令行参数: --load 配置文件
    if len(sys.argv) > 1 and sys.argv[1] == "--load" and len(sys.argv) > 2:
        path = sys.argv[2]
        if os.path.isfile(path):
            try:
                with open(path, "r", encoding="utf-8") as f:
                    data = json.load(f)
                app._load_config_data(data, path)
            except Exception as e:
                print(f"[ERROR] 加载 {path} 失败: {e}", file=sys.stderr)

    root.mainloop()


if __name__ == "__main__":
    main()
