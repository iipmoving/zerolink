// 快速测试脚本 - 在浏览器控制台中运行

// 1. 检查必要对象
console.log('AppState:', typeof AppState);
console.log('AppState.canvas:', AppState?.canvas ? '存在' : '不存在');
console.log('AppState.logicLayer:', AppState?.logicLayer ? AppState.logicLayer.constructor.name : '不存在');
console.log('AppState.controls:', AppState?.controls?.length || 0, '个控件');

// 2. 检查KeyEvent
console.log('KeyEvent:', typeof KeyEvent);
console.log('KeyEvent.KEY_EVENT_PRESS:', KeyEvent?.KEY_EVENT_PRESS);

// 3. 找到"开始 / 暂停"键
const btn = AppState?.controls?.find(c => c.name === '开始 / 暂停');
console.log('开始/暂停键:', btn ? '找到' : '未找到');
if (btn) {
    console.log('  - 位置:', btn.x, btn.y);
    console.log('  - config:', btn.config);
}

// 4. 手动调用pressKey测试
if (AppState?.logicLayer && btn) {
    console.log('\n尝试发送PRESS事件...');
    try {
        AppState.logicLayer.pressKey(13, KeyEvent.KEY_EVENT_PRESS);
        console.log('✅ pressKey调用成功');
        
        setTimeout(() => {
            const state = AppState.logicLayer.getState();
            console.log('当前状态:', state);
        }, 50);
    } catch(e) {
        console.error('❌ pressKey失败:', e.message);
    }
} else {
    console.error('❌ 缺少必要对象');
}
