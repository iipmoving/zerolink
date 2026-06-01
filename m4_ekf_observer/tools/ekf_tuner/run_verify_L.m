%% Wrapper: run verify_L_consistency and save figures
run('verify_L_consistency.m');

% Save all open figures
figs = findall(0, 'Type', 'figure');
for i = 1:length(figs)
    fname = sprintf('verify_L_fig%d.png', i);
    saveas(figs(i), fname);
    fprintf('Saved %s\n', fname);
end
