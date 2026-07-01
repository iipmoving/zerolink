#!/usr/bin/env bash
# =============================================================================
# auto-sync.sh — 自动同步守护脚本
# 同步 Y:\AI\ZEROLINK → D:\WORK\AIGIT\ZEROLINK + GitHub + Gitee
# =============================================================================
# 用法:
#   ./tools/auto-sync.sh                # 启动守护进程（前台）
#   ./tools/auto-sync.sh -d             # 启动守护进程（后台）
#   ./tools/auto-sync.sh -o             # 立即执行一次同步后退出
# =============================================================================

set -euo pipefail

# ---------- 配置 ----------
SOURCE="/y/AI/ZEROLINK"              # 源仓库 (Git Bash 路径)
LOCAL_DEST="/d/WORK/AIGIT/ZEROLINK"  # 本地备份目录
LOCAL_DEST_WIN="D:\\WORK\\AIGIT\\ZEROLINK"

LOCAL_INTERVAL=1800      # 本地同步间隔（秒）→ 30 分钟
REMOTE_INTERVAL=3600     # 远程同步间隔（秒）→ 1 小时

LOG_FILE="/d/WORK/AIGIT/auto-sync.log"
PID_FILE="/d/WORK/AIGIT/auto-sync.pid"
TEST_SYNC_FILE="$SOURCE/test_sync.txt"

GITHUB_REMOTE="origin"   # git remote 名称（指向 NAS/群晖）
NAS_REMOTE="origin"      # 别名：origin 实际指向 NAS（群晖）
GITEE_REMOTE="gitee"     # git remote 名称（指向 Gitee）

# ---------- 函数 ----------

log() {
  local msg="[$(date '+%Y-%m-%d %H:%M:%S')] $*"
  echo "$msg"
  echo "$msg" >> "$LOG_FILE"
}

cleanup() {
  log "[STOP] 已停止"
  rm -f "$PID_FILE"
  exit 0
}

# 检查源目录是否存在
check_source() {
  if [ ! -d "$SOURCE/.git" ]; then
    log "[ERR] 源目录不是 git 仓库: $SOURCE"
    exit 1
  fi
}

# 提交本地变更
commit_local() {
  cd "$SOURCE"

  # 写入时间戳测试文件
  echo "# Auto-sync test $(date)" > "$TEST_SYNC_FILE"

  # 检查是否有变更
  if git diff --quiet && git diff --cached --quiet && [ -z "$(git ls-files --others --exclude-standard)" ]; then
    log "[LOCAL] Y:\\AI 无变化"
    return 1
  fi

  # 排除嵌套 Git 仓库 (如 m4_ekf_observer)，避免 git add -A 因子模块问题失败
  git add -A -- ':!m4_ekf_observer' 2>/dev/null || true
  # 补充添加 -u 中可能遗漏的跟踪文件变更
  git add -u 2>/dev/null || true
  git commit -m "auto-sync: $(date '+%Y-%m-%d %H:%M:%S')" || true
  log "[LOCAL] Y:\\AI 已提交"
  return 0
}

# 同步到本地备份目录 (D:\WORK\AIGIT)
sync_local_backup() {
  log "[LOCAL] 同步文件到 D:\\WORK\\AIGIT..."

  # 先尝试 git push（如果目标是一个 bare repo 或已配置 remote）
  if git push "file:///d/WORK/AIGIT/ZEROLINK" main:WORKING 2>/dev/null; then
    log "[LOCAL] D:\\WORK\\AIGIT git push 成功"
    return 0
  fi

  # 如果 git push 失败，使用 robocopy（非 bare repo 的情况）
  # 排除 .git 目录避免损坏目标仓库
  # 使用 //MIR 等双斜杠避免 Git Bash/MSYS2 将 /MIR 转换为 Windows 路径
  robocopy "$SOURCE" "$LOCAL_DEST_WIN" //MIR //NDL //NFL //NP \
    //XD ".git" "node_modules" 2>&1 >> "$LOG_FILE" || true

  # 在目标目录提交 robocopy 同步的变更
  if [ -d "$LOCAL_DEST/.git" ]; then
    cd "$LOCAL_DEST"
    if ! git diff --quiet; then
      git add -A
      git commit -m "mirror: $(date '+%Y-%m-%d %H:%M:%S')" 2>/dev/null || true
      log "[LOCAL] D:\\WORK\\AIGIT 已提交"
    else
      log "[LOCAL] D:\\WORK\\AIGIT 无变化"
    fi
  fi
}

# 同步到远程 (群晖 + Gitee)
sync_remote() {
  cd "$SOURCE"

  # --- NAS (群晖) ---
  log "[NAS] 开始同步 Y:\\AI -> NAS (WORKING)"
  if git remote get-url "$NAS_REMOTE" &>/dev/null; then
    if git push "$NAS_REMOTE" main:WORKING 2>&1; then
      log "[NAS] NAS 已更新"
    else
      log "[NAS] NAS 推送失败"
    fi
  else
    log "[NAS] remote '$NAS_REMOTE' 未配置"
  fi

  # --- Gitee ---
  log "[GITEE] 开始同步 Y:\\AI -> Gitee (WORKING)"
  if git remote get-url "$GITEE_REMOTE" &>/dev/null; then
    # 先拉取防止分歧
    git pull "$GITEE_REMOTE" main:WORKING --rebase 2>&1 || true
    if git push "$GITEE_REMOTE" main:WORKING 2>&1; then
      log "[GITEE] Gitee 已更新"
    else
      log "[GITEE] Gitee 推送失败"
    fi
  else
    log "[GITEE] remote '$GITEE_REMOTE' 未配置，添加中..."
    git remote add "$GITEE_REMOTE" "git@gitee.com:lipmoving/ai.git"
    log "[GITEE] 已添加 remote '$GITEE_REMOTE'，下次循环将自动推送"
  fi
}

# 一次性同步（-o 模式）
sync_once() {
  check_source
  commit_local || true
  sync_local_backup
  sync_remote
}

# ---------- 主流程 ----------

trap cleanup SIGINT SIGTERM

# 解析参数
DAEMON=false
ONCE=false
while getopts "do" opt; do
  case $opt in
    d) DAEMON=true ;;
    o) ONCE=true ;;
    *) echo "用法: $0 [-d] [-o]"; exit 1 ;;
  esac
done

if $ONCE; then
  sync_once
  exit 0
fi

if $DAEMON; then
  # 后台模式
  nohup "$0" &>/dev/null &
  echo "$!" > "$PID_FILE"
  log "[OK] 守护进程 PID=$(cat $PID_FILE)"
  exit 0
fi

# 前台守护进程模式
echo "$(sh -c 'echo $PPID')" > "$PID_FILE"

log "=============================================="
log "Y:\\AI\\ZEROLINK  ->  D:\\WORK\\AIGIT\\ZEROLINK (WORKING branch)  每 30 分钟"
log "Y:\\AI\\ZEROLINK  ->  群晖 (WORKING)                每 1 小时"
log "Y:\\AI\\ZEROLINK  ->  Gitee (WORKING)                每 1 小时"
log "Y:\\AI\\ZEROLINK  ->  GitHub (WORKING)               每 1 小时"
log "时间间隔可编辑 $(basename "$0") 修改 LOCAL_INTERVAL / REMOTE_INTERVAL"
log "[OK] 守护进程 PID=$$"

check_source

LOCAL_LAST=0
REMOTE_LAST=0
NOW=$(date +%s)

while true; do
  NOW=$(date +%s)

  # 本地同步
  if [ $((NOW - LOCAL_LAST)) -ge $LOCAL_INTERVAL ]; then
    log "[LOCAL] 开始同步 Y:\\AI -> D:\\WORK\\AIGIT"
    commit_local || true
    sync_local_backup
    LOCAL_LAST=$NOW
  fi

  # 远程同步
  if [ $((NOW - REMOTE_LAST)) -ge $REMOTE_INTERVAL ]; then
    sync_remote
    REMOTE_LAST=$NOW
  fi

  sleep 60
done
