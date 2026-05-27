/**
 * 按键通路自动化测试脚本
 * 
 * 直接在浏览器控制台中运行此脚本，自动测试按键功能
 */

(function() {
    console.log('=== 按键通路自动化测试开始 ===');
    
    // 1. 检查必要的全局对象是否存在
    console.log('\n[步骤1] 检查全局对象...');
    const checks = [
        { name: 'AppState', exists: typeof AppState !== 'undefined' },
        { name: 'AppState.canvas', exists: AppState && AppState.canvas !== undefined },
        { name: 'AppState.logicLayer', exists: AppState && AppState.logicLayer !== undefined },
        { name: 'AppState.controls', exists: AppState && AppState.controls !== undefined },
        { name: 'setupCanvasEvents', exists: typeof setupCanvasEvents !== 'undefined' },
        { name: 'findControlByPosition', exists: typeof findControlByPosition !== 'undefined' }
    ];
    
    let allPassed = true;
    checks.forEach(check => {
        const status = check.exists ? '✅' : '❌';
        console.log(`  ${status} ${check.name}: ${check.exists ? '存在' : '不存在'}`);
        if (!check.exists) allPassed = false;
    });
    
    if (!allPassed) {
        console.error('\n❌ 关键对象缺失，测试终止');
        return;
    }
    
    // 2. 检查Canvas事件绑定
    console.log('\n[步骤2] 检查Canvas事件绑定...');
    const canvas = AppState.canvas;
    if (canvas) {
        console.log('  Canvas元素:', canvas);
        console.log('  Canvas尺寸:', canvas.width, 'x', canvas.height);
        
        // 检查是否有mousedown事件监听器
        const hasMousedownListener = canvas.onmousedown !== null || 
                                     canvas.getAttribute('onmousedown') !== null;
        console.log('  mousedown监听器:', hasMousedownListener ? '已绑定' : '未检测到（可能通过addEventListener）');
    }
    
    // 3. 检查控件列表
    console.log('\n[步骤3] 检查控件列表...');
    if (AppState.controls && AppState.controls.length > 0) {
        console.log(`  控件数量: ${AppState.controls.length}`);
        const buttons = AppState.controls.filter(c => c.type === 'button' || c.type === '按键');
        console.log(`  按键数量: ${buttons.length}`);
        
        if (buttons.length > 0) {
            console.log('  第一个按键:', buttons[0]);
            console.log('  - 名称:', buttons[0].name);
            console.log('  - 位置:', buttons[0].x, ',', buttons[0].y);
            console.log('  - 尺寸:', buttons[0].width, 'x', buttons[0].height);
            console.log('  - config:', buttons[0].config);
        }
    } else {
        console.error('  ❌ 没有控件！');
        return;
    }
    
    // 4. 模拟点击"开始 / 暂停"键
    console.log('\n[步骤4] 模拟点击"开始 / 暂停"键...');
    const startPauseBtn = AppState.controls.find(c => c.name === '开始 / 暂停');
    if (!startPauseBtn) {
        console.error('  ❌ 未找到"开始 / 暂停"键');
        return;
    }
    
    console.log('  找到按键:', startPauseBtn);
    
    // 计算点击坐标（按键中心）
    const clickX = startPauseBtn.x + startPauseBtn.width / 2;
    const clickY = startPauseBtn.y + startPauseBtn.height / 2;
    console.log(`  点击坐标: (${clickX}, ${clickY})`);
    
    // 5. 查找被点击的控件
    console.log('\n[步骤5] 测试findControlByPosition...');
    const foundControl = findControlByPosition(clickX, clickY);
    if (foundControl) {
        console.log('  ✅ 找到控件:', foundControl.name);
    } else {
        console.error('  ❌ 未找到控件');
        return;
    }
    
    // 6. 获取keyCode
    console.log('\n[步骤6] 获取keyCode...');
    let keyCode = foundControl.config?.key_code;
    console.log('  config.key_code:', keyCode);
    
    if (keyCode === undefined || keyCode === null) {
        console.log('  key_code不存在，尝试从名称映射...');
        const KEY_NAME_MAP = {
            '开始 / 暂停': 13,
            '加水': 11,
            '加时间': 12,
            'M1': 1, 'M2': 2, 'M3': 3, 'M4': 4, 'M5': 5,
            'M6': 6, 'M7': 7, 'M8': 8, 'M9': 9, 'M10': 10,
            '电源': 0
        };
        keyCode = KEY_NAME_MAP[foundControl.name];
        console.log('  映射后的keyCode:', keyCode);
    }
    
    if (keyCode === undefined) {
        console.error('  ❌ 无法获取keyCode');
        return;
    }
    
    // 7. 检查logicLayer
    console.log('\n[步骤7] 检查logicLayer...');
    if (!AppState.logicLayer) {
        console.error('  ❌ logicLayer不存在');
        return;
    }
    console.log('  logicLayer类型:', AppState.logicLayer.constructor.name);
    console.log('  logicLayer.pressKey方法:', typeof AppState.logicLayer.pressKey);
    
    // 8. 模拟发送按键事件
    console.log('\n[步骤8] 模拟发送PRESS事件...');
    try {
        // 检查KeyEvent常量
        const pressEvent = window.KeyEvent ? KeyEvent.KEY_EVENT_PRESS : 1;
        console.log('  PRESS事件值:', pressEvent);
        
        AppState.logicLayer.pressKey(keyCode, pressEvent);
        console.log('  ✅ pressKey调用成功');
    } catch (error) {
        console.error('  ❌ pressKey调用失败:', error.message);
        return;
    }
    
    // 9. 等待一个周期，检查状态变化
    console.log('\n[步骤9] 等待状态变化...');
    setTimeout(() => {
        const currentState = AppState.logicLayer.getState();
        console.log('  当前状态:', currentState);
        
        if (currentState !== 2) { // S_SHUTDOWN = 2
            console.log('  ✅ 状态已改变！按键通路正常');
        } else {
            console.log('  ⚠️  状态未改变（可能需要在S_SHUTDOWN状态下按特定键才能开机）');
        }
        
        console.log('\n=== 测试完成 ===');
    }, 100);
})();
