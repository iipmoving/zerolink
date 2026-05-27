const puppeteer = require('puppeteer');

(async () => {
    console.log('启动浏览器...');
    const browser = await puppeteer.launch({ headless: false });
    const page = await browser.newPage();
    
    // 监听控制台消息
    page.on('console', msg => console.log('浏览器控制台:', msg.text()));
    
    console.log('打开页面...');
    await page.goto('http://localhost:8080/index.html', { waitUntil: 'networkidle0' });
    
    console.log('等待页面加载完成...');
    await page.waitForSelector('#mode-switch', { timeout: 5000 });
    
    console.log('切换到演示模式...');
    await page.click('#mode-switch');
    await new Promise(r => setTimeout(r, 1000));
    
    console.log('点击"开始 / 暂停"键...');
    // 查找Canvas并点击特定位置（开始/暂停键的位置是99, 276）
    const canvas = await page.$('#main-canvas');
    if (canvas) {
        const box = await canvas.boundingBox();
        console.log('Canvas位置:', box);
        
        // 点击开始/暂停键（相对Canvas的坐标）
        await page.mouse.click(box.x + 99 + 20, box.y + 276 + 33);
        
        console.log('等待状态变化...');
        await new Promise(r => setTimeout(r, 2000));
        
        console.log('测试完成！');
    } else {
        console.error('未找到Canvas元素');
    }
    
    // 保持浏览器打开，让用户查看结果
    console.log('浏览器将保持打开状态，请手动关闭');
})();
