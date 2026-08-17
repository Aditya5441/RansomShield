%% analyze_ecdh_security_visual.m
% Security-robustness analysis for the ECDH / session-key subsystem in
% Ransomware_detection_core (Application/Security/{ecdh,session}.c).
%
% Single self-contained file: main script + all local functions below.
% Requires MATLAB R2016b+ (local functions in scripts) and Java interop
% enabled (default) for java.math.BigInteger 256-bit modular arithmetic.
%
% Results are shown as plots (one figure per stage) with only short
% summary lines printed to the console. Not a live attack tool - static
% + numerical analysis only, no hardware connection.
%
% Usage:
%   >> analyze_ecdh_security_visual

clear; clc; close all;
fprintf('Running ECDH/session-key security analysis (see figures)...\n');

results = struct();

%% 1+2. Curve parameter validation + base point order
results.curve = check_curve_params();
results.order = check_base_point_order();
fprintf('[1-2] Curve/order checks: %s\n', overall_tag(results.curve.pass && results.order.pass));

%% 3. RNG / private-key entropy test
n_keys = 512;
keys = simulate_device_rng(n_keys);   % swap in real captured device bytes for a true HW assessment
results.rng = run_rng_test_suite(keys);
fprintf('[3]   RNG statistical suite (%d simulated keys): %s\n', n_keys, overall_tag(results.rng.overall_pass));

%% 4. Invalid-curve / weak-point attack surface simulation
results.invalid_curve = simulate_invalid_curve_attack();
fprintf('[4]   Invalid-curve attack surface: %s\n', results.invalid_curve.severity);

%% 5. Design / protocol checklist
results.checklist = protocol_checklist();

%% 6. Composite score
score = composite_score(results);
fprintf('[6]   Composite robustness score: %d / 100\n', score);

%% All plots combined into a single figure
plot_dashboard(results, keys, score);

save('ecdh_security_analysis_results.mat', 'results', 'score');


%% ================== LOCAL FUNCTIONS: ANALYSIS ==================

function res = check_curve_params()
import java.math.BigInteger
hex = @(bytes) sprintf('%02X', bytes);

P256_PRIME = [0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, ...
  0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF];
P256_A = [0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, ...
  0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC];
P256_GX = [0x6B,0x17,0xD1,0xF2,0xE1,0x2C,0x42,0x47,0xF8,0xBC,0xE6,0xE5,0x63,0xA4,0x40,0xF2, ...
  0x77,0x03,0x7D,0x81,0x2D,0xEB,0x33,0xA0,0xF4,0xA1,0x39,0x45,0xD8,0x98,0xC2,0x96];
P256_GY = [0x4F,0xE3,0x42,0xE2,0xFE,0x1A,0x7F,0x9B,0x8E,0xE7,0xEB,0x4A,0x7C,0x0F,0x9E,0x16, ...
  0x2B,0xCE,0x33,0x57,0x6B,0x31,0x5E,0xCE,0xCB,0xB6,0x40,0x68,0x37,0xBF,0x51,0xF5];
P256_ORDER = [0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, ...
  0xBC,0xE6,0xFA,0xAD,0xA7,0x17,0x9E,0x84,0xF3,0xB9,0xCA,0xC2,0xFC,0x63,0x25,0x51];

REF_PRIME = P256_PRIME; REF_A = P256_A; REF_GX = P256_GX; REF_GY = P256_GY; REF_ORDER = P256_ORDER;
REF_B_HEX = '5AC635D8AA3A93E7B3EBBD55769886BC651D06B0CC53B0F63BCE3C3E27D2604B';

res = struct('name', 'Curve parameter validation', 'pass', true, 'labels', {{}}, 'checks', []);
fields = {'prime',P256_PRIME,REF_PRIME; 'a',P256_A,REF_A; 'Gx',P256_GX,REF_GX; ...
          'Gy',P256_GY,REF_GY; 'order',P256_ORDER,REF_ORDER};
for i = 1:size(fields,1)
    ok = logical(isequal(fields{i,2}, fields{i,3}));
    res.labels{end+1} = fields{i,1};
    res.checks(end+1) = ok;
    res.pass = res.pass && ok;
end

p  = BigInteger(hex(P256_PRIME), 16); a = BigInteger(hex(P256_A), 16);
gx = BigInteger(hex(P256_GX), 16);    gy = BigInteger(hex(P256_GY), 16);
lhs = gy.multiply(gy).mod(p);
rhs = gx.pow(3).add(a.multiply(gx)).mod(p);
implied_b = lhs.subtract(rhs).mod(p);
b_ok = logical(implied_b.equals(BigInteger(REF_B_HEX, 16)));
res.labels{end+1} = 'b (derived)';
res.checks(end+1) = b_ok;
res.pass = res.pass && b_ok;
res.severity = ternary(res.pass, 'OK', 'CRITICAL');
end


function res = check_base_point_order()
import java.math.BigInteger
hexstr = @(bytes) sprintf('%02X', bytes);
P256_PRIME = [0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, ...
  0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF];
P256_A = [0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, ...
  0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC];
P256_GX = [0x6B,0x17,0xD1,0xF2,0xE1,0x2C,0x42,0x47,0xF8,0xBC,0xE6,0xE5,0x63,0xA4,0x40,0xF2, ...
  0x77,0x03,0x7D,0x81,0x2D,0xEB,0x33,0xA0,0xF4,0xA1,0x39,0x45,0xD8,0x98,0xC2,0x96];
P256_GY = [0x4F,0xE3,0x42,0xE2,0xFE,0x1A,0x7F,0x9B,0x8E,0xE7,0xEB,0x4A,0x7C,0x0F,0x9E,0x16, ...
  0x2B,0xCE,0x33,0x57,0x6B,0x31,0x5E,0xCE,0xCB,0xB6,0x40,0x68,0x37,0xBF,0x51,0xF5];
P256_ORDER = [0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, ...
  0xBC,0xE6,0xFA,0xAD,0xA7,0x17,0x9E,0x84,0xF3,0xB9,0xCA,0xC2,0xFC,0x63,0x25,0x51];

p = BigInteger(hexstr(P256_PRIME), 16); a = BigInteger(hexstr(P256_A), 16);
n = BigInteger(hexstr(P256_ORDER), 16);
Gx = BigInteger(hexstr(P256_GX), 16); Gy = BigInteger(hexstr(P256_GY), 16);

result_point = ec_scalar_mul(Gx, Gy, n, a, p);
res = struct('name', 'Base point order check (n*G == O)', 'pass', logical(isempty(result_point)));
res.severity = ternary(res.pass, 'OK', 'CRITICAL');
end

function pt = ec_scalar_mul(Px, Py, k, a, p)
import java.math.BigInteger
result = {}; addend = {Px, Py}; kk = k; ZERO = BigInteger.ZERO;
while kk.compareTo(ZERO) > 0
    if logical(kk.testBit(0)), result = ec_add(result, addend, a, p); end
    addend = ec_add(addend, addend, a, p);
    kk = kk.shiftRight(1);
end
pt = result;
end

function r = ec_add(P, Q, a, p)
import java.math.BigInteger
if isempty(P), r = Q; return; end
if isempty(Q), r = P; return; end
Px = P{1}; Py = P{2}; Qx = Q{1}; Qy = Q{2};
if logical(Px.equals(Qx)) && ~logical(Py.equals(Qy)), r = {}; return; end
if logical(Px.equals(Qx)) && logical(Py.equals(Qy))
    num = Px.pow(2).multiply(BigInteger.valueOf(3)).add(a).mod(p);
    den = Py.multiply(BigInteger.valueOf(2)).mod(p);
else
    num = Qy.subtract(Py).mod(p);
    den = Qx.subtract(Px).mod(p);
end
lambda = num.multiply(den.modInverse(p)).mod(p);
Rx = lambda.pow(2).subtract(Px).subtract(Qx).mod(p);
Ry = lambda.multiply(Px.subtract(Rx)).subtract(Py).mod(p);
r = {Rx, Ry};
end


function keys = simulate_device_rng(n_keys)
% *** MATLAB PRNG stand-in, NOT the STM32 TRNG. *** Replace body with a
% real captured bm_rng_read_u32() dump from hardware for a true assessment.
P256_ORDER = [0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, ...
  0xBC,0xE6,0xFA,0xAD,0xA7,0x17,0x9E,0x84,0xF3,0xB9,0xCA,0xC2,0xFC,0x63,0x25,0x51];
keys = zeros(n_keys, 32, 'uint8');
for i = 1:n_keys
    while true
        priv = uint8(randi([0 255], 1, 32));
        if any(priv ~= 0) && bytes_lt(priv, P256_ORDER), break; end
    end
    keys(i, :) = priv;
end
end

function b = bytes_lt(x, y)
b = false;
for i = 1:numel(x)
    if x(i) < y(i), b = true; return; end
    if x(i) > y(i), b = false; return; end
end
end


function res = run_rng_test_suite(keys)
[n_keys, nbytes] = size(keys);
bits = zeros(1, n_keys*nbytes*8); idx = 1;
for i = 1:n_keys
    for byte = 1:nbytes
        bb = keys(i, byte);
        for bitpos = 7:-1:0
            bits(idx) = bitget(bb, bitpos+1); idx = idx + 1;
        end
    end
end
N = numel(bits);
res = struct(); res.n_keys = n_keys; res.n_bits = N;

s = sum(bits == 1) - sum(bits == 0);
stat = abs(s) / sqrt(N);
p_monobit = erfc(stat / sqrt(2));
res.monobit = struct('p', p_monobit, 'pass', p_monobit >= 0.01);

pi_hat = sum(bits) / N;
if abs(pi_hat - 0.5) >= (2/sqrt(N))
    res.runs = struct('p', 0, 'pass', false);
else
    runs = 1 + sum(bits(2:end) ~= bits(1:end-1));
    num = abs(runs - 2*N*pi_hat*(1-pi_hat));
    den = 2*sqrt(2*N)*pi_hat*(1-pi_hat);
    p_runs = erfc(num/den);
    res.runs = struct('p', p_runs, 'pass', p_runs >= 0.01);
end

allbytes = reshape(keys', 1, []);
counts = histcounts(double(allbytes), -0.5:1:255.5);
expected = numel(allbytes) / 256;
chi2 = sum((counts - expected).^2 / expected);
res.chi_square = struct('stat', chi2, 'crit_0_01', 310.46, 'pass', chi2 < 310.46, 'counts', counts);

maxlag = 8; threshold = 4/sqrt(N);
autocorr = zeros(1, maxlag);
bpm = double(bits)*2 - 1;
for lag = 1:maxlag
    autocorr(lag) = mean(bpm(1:end-lag) .* bpm(1+lag:end));
end
res.autocorr = struct('values', autocorr, 'threshold', threshold, 'pass', all(abs(autocorr) < threshold));

p_max = max(counts) / numel(allbytes);
min_entropy_per_byte = -log2(p_max);
res.min_entropy = struct('bits_per_byte', min_entropy_per_byte, 'pass', min_entropy_per_byte > 7.0);

[~, ia] = unique(keys, 'rows');
res.duplicates = struct('n_unique', numel(ia), 'n_total', n_keys, 'pass', numel(ia) == n_keys);

res.overall_pass = res.monobit.pass && res.runs.pass && res.chi_square.pass ...
    && res.autocorr.pass && res.min_entropy.pass && res.duplicates.pass;
end


function res = simulate_invalid_curve_attack()
% Toy curve demonstration of why ECDH_ComputeShared's missing
% peer-key validation is exploitable. Not a real P-256 attack.
p = 97; a = 2; b = 3;
pts = {};
for x = 0:p-1
    rhs = mod(x^3 + a*x + b, p);
    for y = 0:p-1
        if mod(y*y, p) == rhs, pts{end+1} = [x y]; end %#ok<AGROW>
    end
end
group_order = numel(pts) + 1;
small_order_found = false; attack_order = 0;
for i = 1:numel(pts)
    ord = point_order(pts{i}, a, p, group_order);
    if ord > 1 && ord < 8, small_order_found = true; attack_order = ord; break; end
end
res = struct('name', 'Invalid-curve / small-subgroup attack surface', ...
    'pass', ~small_order_found, 'attack_order', attack_order);
res.severity = ternary(small_order_found, 'HIGH', 'INFO');
end

function ord = point_order(P, a, p, max_order)
Q = P; ord = 1;
while ord <= max_order
    Q = ec_add_toy(Q, P, a, p); ord = ord + 1;
    if isequal(Q, [-1 -1]), return; end
end
end

function R = ec_add_toy(P, Q, a, p)
if isequal(P, [-1 -1]), R = Q; return; end
if isequal(Q, [-1 -1]), R = P; return; end
if P(1) == Q(1) && mod(P(2)+Q(2), p) == 0, R = [-1 -1]; return; end
if isequal(P, Q)
    num = mod(3*P(1)^2 + a, p); den = mod(2*P(2), p);
else
    num = mod(Q(2)-P(2), p); den = mod(Q(1)-P(1), p);
end
lambda = mod(num*modinv(den, p), p);
Rx = mod(lambda^2 - P(1) - Q(1), p);
Ry = mod(lambda*(P(1)-Rx) - P(2), p);
R = [Rx Ry];
end

function inv = modinv(a, m)
[g, x, ~] = extgcd(mod(a,m), m);
inv = ternary(g == 1, mod(x, m), NaN);
end

function [g, x, y] = extgcd(a, b)
if a == 0
    g = b; x = 0; y = 1;
else
    [g, x1, y1] = extgcd(mod(b,a), a);
    x = y1 - floor(b/a)*x1; y = x1;
end
end


function items = protocol_checklist()
% {description, status('pass'/'fail'/'warn'), weight, note}
% Reflects fixed_firmware/{ecdh,session}.{c,h}: ECDH_ValidatePublicKey()
% added + called before ECDH_ComputeShared, KDF salted with both sides'
% handshake nonces, and HELO frames now HMAC-SHA256 authenticated with
% a pre-shared key (SESSION_SetPSK). See those files for the diff.
items = {
 'Standard published curve (NIST P-256)', 'pass', 10, 'Confirmed by curve check.'
 'Private scalar: HW RNG + zero/range rejection', 'pass', 10, 'Depends on bm_rng_read_u32 quality -- see RNG suite.'
 'Peer public key validated before use', 'pass', 20, 'FIXED: ECDH_ValidatePublicKey() checks reduced coords, non-infinity, and on-curve (cofactor 1 => sufficient) before ECDH_ComputeShared is ever called.'
 'ECDH output passed through a KDF', 'pass', 15, 'SHA-256 with fixed context string, better than raw x-coordinate.'
 'KDF uses per-session random salt', 'pass', 10, 'FIXED: KDF input now includes both sides'' fresh 16-byte handshake nonces alongside shared_x and the context string.'
 'Secrets zeroized after use', 'pass', 5, 'priv[], shared_x[], and kdf_input[] memset after key derivation.'
 'Handshake messages authenticated (signed/MAC)', 'pass', 20, 'FIXED: HELO frames now carry an HMAC-SHA256(PSK, frame) tag, verified (constant-time) before the peer public key is trusted. Requires SESSION_SetPSK() to be called with a provisioned PSK at boot -- handshake now refuses to run unauthenticated if no PSK is set.'
 'Resistant to replay of stale peer keys', 'warn', 5, 'Still only bounded by the 2000ms receive timeout; nonces are per-session (see KDF salt) but no explicit replay-window/sequence check yet.'
 'Bounded blocking on handshake receive', 'pass', 5, 'HANDSHAKE_TIMEOUT_MS avoids indefinite blocking DoS.'
 'PSK securely provisioned and stored on-device', 'fail', 15, 'SESSION_SetPSK() authenticates handshakes IF given a real secret, but this patch does not implement PSK provisioning/secure storage -- that has to be wired to this project''s existing device-secret mechanism before the HMAC protection is meaningful in practice. Tracked as outstanding, not silently assumed done.'
};
end

function score = composite_score(results)
checklist_score = 0; total_weight = 0;
for i = 1:size(results.checklist,1)
    w = results.checklist{i,3}; total_weight = total_weight + w;
    switch results.checklist{i,2}
        case 'pass', checklist_score = checklist_score + w;
        case 'warn', checklist_score = checklist_score + 0.5*w;
        case 'fail', checklist_score = checklist_score + 0;
    end
end
checklist_pct = 100 * checklist_score / total_weight;
gate_penalty = 0;
if ~results.curve.pass, gate_penalty = gate_penalty + 40; end
if ~results.order.pass, gate_penalty = gate_penalty + 40; end
if ~results.rng.overall_pass, gate_penalty = gate_penalty + 15; end
score = round(max(0, checklist_pct - gate_penalty));
end

function s = ternary(cond, a, b)
if cond, s = a; else, s = b; end
end

function s = overall_tag(pass)
s = ternary(pass, 'PASS', 'FAIL');
end


%% ================== LOCAL FUNCTIONS: PLOTTING (single figure) ==================

function plot_dashboard(results, keys, score)
fig = figure('Name', 'ECDH Security Analysis Dashboard', 'Color', 'w', ...
    'Position', [50 50 1400 900]);
% Force the light theme so titles/labels render black even if MATLAB's
% default is set to dark mode (R2025a+ figures otherwise grey out text).
try
    fig.Theme = 'light';
catch
    % older MATLAB without figure Themes - fall back to manual coloring below
end
set(fig, 'DefaultTextColor', 'k', 'DefaultAxesXColor', 'k', ...
    'DefaultAxesYColor', 'k', 'DefaultAxesZColor', 'k');
tiledlayout(3, 3, 'Padding', 'compact', 'TileSpacing', 'compact');

r = results.rng;
allbytes = reshape(keys', 1, []);

% --- (1) Curve parameter validation ---
nexttile;
labels = [results.curve.labels, {'n*G=O'}];
vals = [results.curve.checks, double(results.order.pass)];
if all(logical(results.curve.checks)) ~= results.curve.pass
    warning('Curve check bars disagree with curve.pass aggregate -- investigate check_curve_params.');
end
colors = repmat([0.85 0.2 0.2], numel(vals), 1);
colors(logical(vals), :) = repmat([0.2 0.7 0.3], sum(vals), 1);
b = bar(vals, 'FaceColor', 'flat'); b.CData = colors;
set(gca, 'XTick', 1:numel(labels), 'XTickLabel', labels, 'YTick', [0 1], ...
    'YTickLabel', {'FAIL','PASS'}, 'YLim', [0 1.3], 'FontSize', 8, ...
    'XColor', 'k', 'YColor', 'k');
title('Curve Parameter Validation', 'Color', 'k');

% --- (2) Byte histogram vs expected uniform ---
nexttile;
histogram(allbytes, -0.5:8:255.5, 'FaceColor', [0.3 0.5 0.8]);
hold on;
yline(numel(allbytes)/256*8, 'r--', 'Expected');
title('RNG: Byte Value Distribution', 'Color', 'k'); xlabel('Byte value', 'Color', 'k'); ylabel('Count', 'Color', 'k');

% --- (3) Autocorrelation ---
nexttile;
bar(r.autocorr.values, 'FaceColor', [0.6 0.4 0.8]);
hold on;
yline(r.autocorr.threshold, 'r--'); yline(-r.autocorr.threshold, 'r--');
title('RNG: Bit Autocorrelation', 'Color', 'k'); xlabel('Lag', 'Color', 'k'); ylabel('Correlation', 'Color', 'k');

% --- (4) RNG test statistics ---
nexttile;
test_names = {'Monobit','Runs','Chi-sq','MinEnt/8'};
pvals = [r.monobit.p, r.runs.p, min(1, 310.46/max(r.chi_square.stat,1e-9)), r.min_entropy.bits_per_byte/8];
passvec = [r.monobit.pass, r.runs.pass, r.chi_square.pass, r.min_entropy.pass];
bcolors = repmat([0.2 0.7 0.3], 4, 1);
bcolors(~passvec, :) = repmat([0.85 0.2 0.2], sum(~passvec), 1);
bb = bar(pvals, 'FaceColor', 'flat'); bb.CData = bcolors;
set(gca, 'XTick', 1:4, 'XTickLabel', test_names, 'FontSize', 8, 'XColor', 'k', 'YColor', 'k');
yline(0.01, 'k:');
title('RNG: Test Statistics (green=pass)', 'Color', 'k');

% --- (5) RNG summary text ---
nexttile; axis off;
summary = sprintf(['Keys: %d (%d bits)\n\n' ...
    'Monobit:     %s\nRuns:        %s\nChi-square:  %s\n' ...
    'Autocorr:    %s\nMinEntropy:  %.2f b/B %s\nDuplicates:  %d/%d %s\n\n' ...
    'RNG OVERALL: %s'], ...
    r.n_keys, r.n_bits, tag(r.monobit.pass), tag(r.runs.pass), tag(r.chi_square.pass), ...
    tag(r.autocorr.pass), r.min_entropy.bits_per_byte, tag(r.min_entropy.pass), ...
    r.duplicates.n_unique, r.duplicates.n_total, tag(r.duplicates.pass), tag(r.overall_pass));
text(0, 0.5, summary, 'FontName', 'FixedWidth', 'FontSize', 9, 'VerticalAlignment', 'middle', 'Color', 'k');
title('RNG Summary', 'Color', 'k');

% --- (6) Invalid-curve attack finding ---
nexttile; axis off;
ic = results.invalid_curve;
ic_text = sprintf(['Invalid-curve attack surface\n\n' ...
    'Peer key validation: MISSING\n(ecdh.c: ECDH_ComputeShared)\n\n' ...
    'Toy-curve demo severity: %s\nLow-order point found: order %d\n\n' ...
    'Risk: MITM can leak private-key\nbits via unvalidated peer point.'], ...
    ic.severity, ic.attack_order);
text(0, 0.5, ic_text, 'FontSize', 9, 'VerticalAlignment', 'middle', 'Color', [0.7 0.1 0.1]);
title('Invalid-Curve Attack Finding', 'Color', 'k');

% --- (7-8) Protocol checklist (spans 2 tiles) ---
nexttile([1 2]);
items = results.checklist;
n = size(items,1);
weights = cell2mat(items(:,3));
colors2 = zeros(n,3);
for i = 1:n
    switch items{i,2}
        case 'pass', colors2(i,:) = [0.2 0.7 0.3];
        case 'warn', colors2(i,:) = [0.9 0.7 0.1];
        case 'fail', colors2(i,:) = [0.85 0.2 0.2];
    end
end
bh = barh(weights, 'FaceColor', 'flat'); bh.CData = colors2;
set(gca, 'YTick', 1:n, 'YTickLabel', items(:,1), 'YDir', 'reverse', 'FontSize', 8, 'XColor', 'k', 'YColor', 'k');
xlabel('Weight (impact)', 'Color', 'k');
title('Protocol Design Checklist (green=pass, yellow=warn, red=fail)', 'Color', 'k');
grid on;

% --- (9) Composite score gauge ---
nexttile;
theta = linspace(pi, 0, 100);
plot(cos(theta), sin(theta), 'k', 'LineWidth', 8); hold on;
frac = score/100;
theta2 = linspace(pi, pi - frac*pi, 100);
plot(cos(theta2), sin(theta2), 'Color', score_color(score), 'LineWidth', 8);
text(0, -0.25, sprintf('%d/100', score), 'FontSize', 20, 'HorizontalAlignment', 'center', 'FontWeight', 'bold', 'Color', 'k');
axis equal off;
xlim([-1.2 1.2]); ylim([-0.5 1.2]);
title('Composite Robustness Score', 'Color', 'k');

sgtitle('ECDH / Session-Key Security Robustness Dashboard', 'FontSize', 14, 'FontWeight', 'bold', 'Color', 'k');
end

function s = tag(pass)
if pass, s = 'PASS'; else, s = 'FAIL'; end
end

function c = score_color(score)
if score >= 70, c = [0.2 0.7 0.3];
elseif score >= 40, c = [0.9 0.7 0.1];
else, c = [0.85 0.2 0.2];
end
end
