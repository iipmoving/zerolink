# ZEROLINK 远程仓库连接手册

## 一、三个远程仓库

| 远程名 | 地址 | 用途 |
|--------|------|------|
| `origin` | GitHub | 主要远程备份 |
| `gitee` | Gitee | 国内备份 |
| `synology` | 群晖 NAS | 本地内网主仓库（自动同步到 GitHub） |

**查看当前配置：**
```bash
git remote -v
```

## 二、连接信息

### GitHub
```
地址: https://github.com/iipmoving/zerolink.git
用户: iipmoving
Token: YOUR_GITHUB_TOKEN_HERE
远程名: origin
```

### Gitee
```
地址: https://gitee.com/lipmoving/ai.git
用户: 18923805256
密码: YOUR_GITEE_PASSWORD_HERE
远程名: gitee
```

### 群晖 Synology
```
地址: moving@100.100.39.64:/volume1/git/zerolink-methodology.git
SSH: 免密登录（密钥已配置）
远程名: synology
```

## 三、推送命令

```bash
git push origin main        # 推送到 GitHub
git push gitee main         # 推送到 Gitee
git push synology main      # 推送到群晖
```

## 四、自动同步

群晖配置了 `post-receive hook`，**推送到群晖后自动同步到 GitHub**：
```
电脑 ──push──▶ 群晖 ──hook──▶ GitHub
```

因此日常只需：
```bash
git push synology main      # 推群晖 → 自动到 GitHub
git push gitee main          # 手动推 Gitee
```

## 五、首次从任意远程克隆

```bash
# 从 GitHub
git clone https://github.com/iipmoving/zerolink.git

# 从 Gitee
git clone https://gitee.com/lipmoving/ai.git

# 从群晖（需在群晖同一内网或 Tailscale 下）
git clone moving@100.100.39.64:/volume1/git/zerolink-methodology.git
```

## 六、添加远程（如果新克隆时丢失）

```bash
git remote add github https://github.com/iipmoving/zerolink.git
git remote add gitee https://YOUR_GITEE_USER:YOUR_GITEE_PASSWORD@gitee.com/lipmoving/ai.git
git remote add synology moving@100.100.39.64:/volume1/git/zerolink-methodology.git
```
