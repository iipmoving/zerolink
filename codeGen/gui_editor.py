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
import subprocess


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

    # ----- 右侧 Notebook (管道编辑 + 代码预览) -----
    def _build_editor_notebook(self, parent):
        self.notebook = ttk.Notebook(parent)
        self.notebook.pack(fill=tk.BOTH, expand=True)

        # Tab 1: 管道 + 字段编辑
        self._build_pipe_tab()

        # Tab 2: 代码预览
        self._build_preview_tab()

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

        # -- 字段编辑 --
        ff = ttk.LabelFrame(tab, text="字段列表", padding=6)
        ff.pack(fill=tk.BOTH, expand=True)

        toolbar = ttk.Frame(ff)
        toolbar.pack(fill=tk.X, pady=(0, 4))
        ttk.Button(toolbar, text="+ 字段", command=self.cmd_add_field, width=8).pack(side=tk.LEFT, padx=2)
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

    def _build_preview_tab(self):
        tab = ttk.Frame(self.notebook)
        self.notebook.add(tab, text="代码预览")

        self.preview_text = tk.Text(tab, wrap=tk.NONE, font=("Consolas", 10), bg="#f5f5f5")
        self.preview_text.pack(fill=tk.BOTH, expand=True)
        self.preview_text.config(state=tk.DISABLED)

        hsb = ttk.Scrollbar(tab, orient=tk.HORIZONTAL, command=self.preview_text.xview)
        hsb.pack(side=tk.BOTTOM, fill=tk.X)
        vsb = ttk.Scrollbar(tab, orient=tk.VERTICAL, command=self.preview_text.yview)
        vsb.pack(side=tk.RIGHT, fill=tk.Y)
        self.preview_text.configure(xscrollcommand=hsb.set, yscrollcommand=vsb.set)

    # ================================================================
    # 底部按钮栏
    # ================================================================
    def _build_bottom_bar(self):
        bar = ttk.Frame(self.root)
        bar.pack(fill=tk.X, padx=4, pady=(0, 4))

        ttk.Button(bar, text="打开 JSON", command=self.cmd_open).pack(side=tk.LEFT, padx=2)
        ttk.Button(bar, text="保存 JSON", command=self.cmd_save).pack(side=tk.LEFT, padx=2)
        ttk.Separator(bar, orient=tk.VERTICAL).pack(side=tk.LEFT, fill=tk.Y, padx=6)
        ttk.Button(bar, text="全部生成", command=lambda: self.cmd_generate("all")).pack(side=tk.LEFT, padx=2)
        ttk.Button(bar, text="仅 io.h", command=lambda: self.cmd_generate("io")).pack(side=tk.LEFT, padx=2)
        ttk.Button(bar, text="仅 switcher", command=lambda: self.cmd_generate("switcher")).pack(side=tk.LEFT, padx=2)
        ttk.Button(bar, text="仅 modules", command=lambda: self.cmd_generate("modules")).pack(side=tk.LEFT, padx=2)

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

        # 填充项目设置
        proj = data.get("project", {})
        paths = proj.get("paths", {})
        self.entry_output_root.delete(0, tk.END)
        self.entry_output_root.insert(0, proj.get("output_root", ""))
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
        self._update_preview("")
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
            "output_root": self.entry_output_root.get(),
        }
        return {
            "schema_version": "1.0",
            "project": project,
            "modules": self.modules,
            "pipes": self.pipes,
            "slot_order": self.slot_order,
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

    def _on_tree_select(self, event):
        sel = self.tree.selection()
        if not sel:
            return
        item = sel[0]
        vals = self.tree.item(item, "values")
        if not vals or vals[0] != "pipe":
            self._clear_pipe_editor()
            return

        pipe_id = vals[1]
        pipe = next((p for p in self.pipes if p["id"] == pipe_id), None)
        if not pipe:
            return

        self._selected_pipe = pipe
        self._show_pipe(pipe)
        self._refresh_field_table(pipe)
        self._show_pipe_preview(pipe)

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
    def _refresh_field_table(self, pipe):
        self.field_tree.delete(*self.field_tree.get_children())
        for f in pipe.get("fields", []):
            self.field_tree.insert("", tk.END, values=(f.get("name", ""), f.get("type", ""), f.get("comment", "")))

    def cmd_add_field(self):
        if not self._selected_pipe:
            messagebox.showinfo("提示", "请先选择一个管道")
            return
        fields = self._selected_pipe.setdefault("fields", [])
        fields.append({"name": "new_field", "type": "uint16_t", "comment": ""})
        self._refresh_field_table(self._selected_pipe)
        self._show_pipe_preview(self._selected_pipe)

    def cmd_delete_field(self):
        sel = self.field_tree.selection()
        if not sel or not self._selected_pipe:
            return
        idx = self.field_tree.index(sel[0])
        fields = self._selected_pipe.get("fields", [])
        if 0 <= idx < len(fields):
            fields.pop(idx)
        self._refresh_field_table(self._selected_pipe)
        self._show_pipe_preview(self._selected_pipe)

    def _on_field_double_click(self, event):
        if not self._selected_pipe:
            return
        sel = self.field_tree.selection()
        if not sel:
            return
        idx = self.field_tree.index(sel[0])
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
            fields = self._selected_pipe.get("fields", [])
            if 0 <= idx < len(fields):
                fields[idx][key] = new_val
            self._show_pipe_preview(self._selected_pipe)

        def _on_cancel(event=None):
            entry.destroy()

        entry.bind("<Return>", _on_confirm)
        entry.bind("<Escape>", _on_cancel)
        entry.bind("<FocusOut>", _on_confirm)

    # ================================================================
    # 代码预览
    # ================================================================
    def _update_preview(self, text: str):
        self.preview_text.config(state=tk.NORMAL)
        self.preview_text.delete(1.0, tk.END)
        self.preview_text.insert(1.0, text)
        self.preview_text.config(state=tk.DISABLED)

    def _show_pipe_preview(self, pipe):
        """生成选中管道的 io.h 片段预览"""
        if not pipe:
            self._update_preview("")
            return
        from_name = pipe["from"]
        to_name = pipe["to"]
        fields = pipe.get("fields", [])
        link_style = pipe.get("out_link", {}).get("style", "array")
        arr_size = pipe.get("out_link", {}).get("array_size", 4)

        lines = []
        lines.append("// ====== OUTPUT_PARAMS (in From_io.h) ======")
        lines.append(f"/* {from_name} → {to_name} 输出参数 */")
        lines.append(f"typedef struct {{")
        lines.append(f"    LINK({from_name}, {to_name});")
        if link_style == "array":
            lines.append(f"    /* PARAMS({from_name}, {to_name}) — 数组 x{arr_size} */")
            lines.append(f"    PARAMS({from_name}, {to_name}) params[{arr_size}];")
        elif link_style == "pointer":
            lines.append(f"    /* PARAMS({from_name}, {to_name}) — 指针 */")
            lines.append(f"    PARAMS({from_name}, {to_name}) *params;")
        elif link_style == "pointer_array":
            lines.append(f"    /* PARAMS({from_name}, {to_name}) — 指针数组 x{arr_size} */")
            lines.append(f"    PARAMS({from_name}, {to_name}) *params[{arr_size}];")
        lines.append(f"}} MODULE_OUTPUT_PARAMS({from_name}, {to_name});")
        lines.append("")
        lines.append("// ====== PARAMS 字段 ======")
        if fields:
            lines.append(f"typedef struct {{")
            for f in fields:
                ft = f.get("type", "uint16_t")
                fn = f.get("name", "field")
                fc = f.get("comment", "")
                if fc:
                    lines.append(f"    {ft} {fn};  /* {fc} */")
                else:
                    lines.append(f"    {ft} {fn};")
            lines.append(f"}} {from_name}_{to_name}_Params_t;")
        else:
            lines.append("(暂无字段)")
        lines.append("")
        lines.append("// ====== INPUT_PARAMS (in To_io.h) ======")
        lines.append("/* 与 OUTPUT_PARAMS 对称 — 字段相同 */")

        self._update_preview("\n".join(lines))

    # ================================================================
    # 生成
    # ================================================================
    def cmd_generate(self, mode: str):
        """调用 code_gen.py 生成代码"""
        if not self.config:
            messagebox.showinfo("提示", "请先打开或创建配置文件")
            return

        # 自动保存到临时文件
        tmp_path = os.path.join(os.path.dirname(self.config_path or "."), ".gen_cache.json")
        self._write_config(tmp_path)

        # 确定输出根目录
        output_root = self.entry_output_root.get()
        if not output_root:
            messagebox.showerror("错误", "请填写输出根目录")
            return

        # 构建命令
        script_dir = os.path.dirname(os.path.abspath(__file__))
        gen_py = os.path.join(script_dir, "code_gen.py")

        cmd = [sys.executable, gen_py, "gen", "--config", tmp_path, "--output", output_root]

        if mode == "io":
            cmd.append("--only-io")
        elif mode == "switcher":
            cmd.append("--only-switcher")
        elif mode == "modules":
            cmd.append("--only-modules")

        self.status(f"正在生成 ({mode})...")
        self.root.update()

        try:
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
            if result.returncode == 0:
                self.status(f"生成完成 ({mode})")
                msg = result.stdout.strip()
                messagebox.showinfo("生成结果", msg or "成功")
            else:
                self.status("生成失败")
                err = result.stderr.strip() or result.stdout.strip() or "未知错误"
                messagebox.showerror("生成失败", err)
        except subprocess.TimeoutExpired:
            messagebox.showerror("错误", "生成超时 (30s)")
        except Exception as e:
            messagebox.showerror("错误", str(e))

    # ================================================================
    # 对话框
    # ================================================================
    def cmd_about(self):
        messagebox.showinfo("关于", "v2.3 LINK+PARAMS 配置编辑器\n\n"
                                   "根据 JSON 配置自动生成:\n"
                                   "  - include/*_io.h\n"
                                   "  - core/data_switcher.c\n"
                                   "  - {layer}/*.c / .h")


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
