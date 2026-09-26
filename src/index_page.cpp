#include "index_page.hpp"

namespace {

constexpr std::string_view PAGE = R"HTML(<!doctype html>
<html lang="ru">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Ценники</title>
<style>
:root {
  --bg: #f6f5f2; --panel: #ffffff; --text: #22211f; --muted: #75716a; --border: #e3e0da; --row: #faf9f7;
  --accent: #2f6f5e; --accent-text: #ffffff; --accent-soft: #e4efeb; --danger: #b3402f; --danger-soft: #f7e6e3;
  --focus: 0 0 0 3px rgba(47,111,94,.25);
}
@media (prefers-color-scheme: dark) {
  :root {
    --bg: #161615; --panel: #1f1f1d; --text: #e8e6e1; --muted: #9a968e; --border: #33322f; --row: #252523;
    --accent: #6fbfa6; --accent-text: #10201b; --accent-soft: #1f302a; --danger: #ef8a78; --danger-soft: #3a2320;
    --focus: 0 0 0 3px rgba(111,191,166,.3);
  }
}
* { box-sizing: border-box; }
body { margin: 0; background: var(--bg); color: var(--text);
  font: 14px/1.45 system-ui, -apple-system, "Segoe UI", Roboto, "Noto Sans", sans-serif; }
.bar { background: var(--panel); border-bottom: 1px solid var(--border); }
.bar-in { max-width: 1100px; margin: 0 auto; padding: 0 16px; display: flex; align-items: stretch; gap: 32px; }
.brand { display: flex; align-items: center; gap: 10px; font-size: 17px; font-weight: 650; padding: 14px 0; }
.brand svg { color: var(--accent); }
.tabs { display: flex; gap: 4px; }
.tab { border: none; background: none; color: var(--muted); font: inherit; font-weight: 600; padding: 0 14px; cursor: pointer;
  border-bottom: 2px solid transparent; }
.tab:hover { color: var(--text); }
.tab.active { color: var(--accent); border-bottom-color: var(--accent); }
main { max-width: 1100px; margin: 0 auto; padding: 24px 16px 48px; }
section[hidden] { display: none; }
.toolbar { display: flex; flex-wrap: wrap; align-items: center; gap: 10px 12px; margin-bottom: 14px; }
.toolbar h1 { font-size: 20px; margin: 0 8px 0 0; font-weight: 650; }
.count { color: var(--muted); }
.grow { flex: 1; }
input, select { font: inherit; color: var(--text); background: var(--panel); border: 1px solid var(--border); border-radius: 6px;
  padding: 6px 9px; min-width: 0; }
input:focus, select:focus, button:focus-visible { outline: none; box-shadow: var(--focus); border-color: var(--accent); }
input.bad { border-color: var(--danger); }
.search { width: 260px; }
.btn { font: inherit; font-weight: 600; border-radius: 6px; padding: 6px 12px; cursor: pointer; border: 1px solid var(--border);
  background: var(--panel); color: var(--text); white-space: nowrap; }
.btn:hover { border-color: var(--muted); }
.btn.primary { background: var(--accent); border-color: var(--accent); color: var(--accent-text); }
.btn.primary:hover { filter: brightness(1.08); }
.btn.link { border-color: transparent; background: none; color: var(--muted); padding: 6px 8px; }
.btn.link:hover { color: var(--text); }
.btn.link.danger:hover { color: var(--danger); }
.btn:disabled { opacity: .5; cursor: default; }
.card { background: var(--panel); border: 1px solid var(--border); border-radius: 8px; overflow-x: auto; }
table { width: 100%; border-collapse: collapse; }
th { text-align: left; font-size: 12px; font-weight: 600; color: var(--muted); text-transform: uppercase; letter-spacing: .03em;
  padding: 9px 12px; border-bottom: 1px solid var(--border); white-space: nowrap; }
td { padding: 7px 12px; border-bottom: 1px solid var(--border); vertical-align: middle; }
tr:last-child td { border-bottom: none; }
tbody tr:hover td { background: var(--row); }
tr.editing td { background: var(--accent-soft); }
td input { width: 100%; }
td.num, th.num { text-align: right; font-variant-numeric: tabular-nums; white-space: nowrap; }
td.num input { text-align: right; }
td.muted { color: var(--muted); }
td.actions { width: 1%; white-space: nowrap; text-align: right; }
.empty { padding: 32px 12px; text-align: center; color: var(--muted); }
#toast { position: fixed; left: 50%; bottom: 24px; transform: translateX(-50%); max-width: calc(100% - 32px); padding: 10px 16px;
  border-radius: 8px; background: var(--danger-soft); color: var(--danger); border: 1px solid var(--danger); font-weight: 600;
  box-shadow: 0 4px 16px rgba(0,0,0,.12); }
#toast.ok { background: var(--accent-soft); color: var(--accent); border-color: var(--accent); }
#toast[hidden] { display: none; }
.print-layout { display: grid; grid-template-columns: 340px minmax(0, 1fr); gap: 20px; align-items: start; }
.panel { background: var(--panel); border: 1px solid var(--border); border-radius: 8px; padding: 16px; display: grid; gap: 16px; }
.field { display: grid; gap: 6px; }
.field > span { font-size: 12px; font-weight: 600; color: var(--muted); text-transform: uppercase; letter-spacing: .03em; }
.presets { display: flex; flex-wrap: wrap; gap: 6px; }
.presets .btn.active { background: var(--accent-soft); border-color: var(--accent); color: var(--accent); }
.grid-inputs { display: flex; align-items: center; gap: 8px; color: var(--muted); }
.grid-inputs input { width: 64px; }
.scale { display: flex; align-items: center; gap: 10px; }
.scale input[type=range] { flex: 1; accent-color: var(--accent); padding: 0; border: none; background: none; }
.scale output { width: 44px; text-align: right; font-variant-numeric: tabular-nums; }
.hint { color: var(--muted); font-size: 12px; }
.picker { position: relative; }
.picker input { width: 100%; }
.suggest { position: absolute; left: 0; right: 0; top: calc(100% + 4px); z-index: 5; background: var(--panel); border: 1px solid var(--border);
  border-radius: 6px; box-shadow: 0 6px 20px rgba(0,0,0,.12); max-height: 280px; overflow-y: auto; }
.suggest[hidden] { display: none; }
.suggest button { display: flex; width: 100%; gap: 8px; justify-content: space-between; border: none; background: none; color: var(--text);
  font: inherit; text-align: left; padding: 7px 10px; cursor: pointer; }
.suggest button:hover, .suggest button.cur { background: var(--accent-soft); }
.suggest .price { color: var(--muted); white-space: nowrap; font-variant-numeric: tabular-nums; }
.suggest .none { padding: 7px 10px; color: var(--muted); }
.chosen { list-style: none; margin: 0; padding: 0; border: 1px solid var(--border); border-radius: 6px; max-height: 320px; overflow-y: auto; }
.chosen li { display: flex; align-items: center; gap: 8px; padding: 5px 4px 5px 10px; border-bottom: 1px solid var(--border); }
.chosen li:last-child { border-bottom: none; }
.chosen .nm { flex: 1; min-width: 0; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; }
.chosen .price { color: var(--muted); font-variant-numeric: tabular-nums; white-space: nowrap; }
.chosen .empty { padding: 16px 10px; }
.row-btns { display: flex; gap: 8px; flex-wrap: wrap; }
.print-go { width: 100%; padding: 10px; font-size: 15px; }
.preview-head { display: flex; align-items: baseline; gap: 12px; margin-bottom: 10px; color: var(--muted); }
.preview-head h1 { font-size: 20px; margin: 0; font-weight: 650; color: var(--text); }
.sheets { display: grid; gap: 24px; justify-content: start; }
.sheet { width: 210mm; height: 296mm; padding: 8mm; background: #fff; color: #000; box-shadow: 0 2px 12px rgba(0,0,0,.18);
  display: grid; grid-template-columns: repeat(var(--cols), minmax(0, 1fr)); grid-template-rows: repeat(var(--rows), minmax(0, 1fr)); }
.tag { container-type: size; border: .3mm dashed #999; margin: -.15mm; overflow: hidden; }
.tag-in { height: 100%; display: flex; flex-direction: column; padding: 4cqmin 5cqmin; gap: 2.5cqh;
  font-family: "Noto Sans", "DejaVu Sans", Arial, sans-serif; }
.tag-org { flex: none; text-align: center; font-size: clamp(5px, min(7cqh, 5cqw), 5mm); font-weight: 600; text-transform: uppercase; letter-spacing: .02em;
  border-bottom: .3mm solid #000; padding-bottom: 1.5cqh; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }
.tag-name-box { flex: 1 1 0; min-height: 0; overflow: hidden; display: flex; align-items: center; justify-content: center; text-align: center; }
.tag-name { font-size: calc(min(10cqh, 7cqw) * var(--name-scale, 1)); font-weight: 600; line-height: 1.15; overflow: hidden; display: -webkit-box;
  -webkit-box-orient: vertical; -webkit-line-clamp: 2; overflow-wrap: anywhere; }
.tag-row { flex: none; display: flex; align-items: flex-end; justify-content: space-between; gap: 3cqw; }
.tag-unit { font-size: min(7cqh, 5cqw); color: #333; white-space: nowrap; padding-bottom: .5cqh; }
.tag-price { --fs: min(34cqh, calc(66cqw / var(--len, 3))); display: flex; align-items: flex-start; line-height: .9; font-weight: 800; }
.tag-price .rub { font-size: var(--fs); letter-spacing: -.02em; font-variant-numeric: tabular-nums; }
.tag-price .kop { font-size: calc(var(--fs) * .45); margin-left: 1cqw; font-variant-numeric: tabular-nums; }
.tag-price .cur { font-size: calc(var(--fs) * .36); font-weight: 600; margin-left: 1.5cqw; align-self: flex-end; line-height: 1.3; }
.tag-foot { flex: none; display: flex; justify-content: space-between; gap: 4cqw; font-size: min(6.5cqh, 4.5cqw); border-top: .2mm solid #000;
  padding-top: 1.5cqh; white-space: nowrap; }
.tag-foot .sign { flex: 1; display: flex; gap: 1cqw; max-width: 60%; }
.tag-foot .sign i { flex: 1; border-bottom: .2mm solid #000; }
@page { size: A4 portrait; margin: 0; }
@media print {
  body { background: #fff; }
  .bar, .toolbar, .panel, .preview-head, #toast, section:not(#print) { display: none !important; }
  main { max-width: none; padding: 0; }
  .print-layout { display: block; }
  .sheets { zoom: 1; gap: 0; }
  .sheet { box-shadow: none; break-after: page; }
  .sheet:last-child { break-after: auto; }
}
@media screen { .sheets { zoom: .5; } }
@media screen and (max-width: 900px) { .print-layout { grid-template-columns: minmax(0, 1fr); } .sheets { zoom: .4; } }
@media (max-width: 640px) {
  .bar-in { flex-direction: column; gap: 0; }
  .brand { padding-bottom: 6px; }
  .tab { padding: 10px 12px; }
  .search { width: 100%; }
}
</style>
</head>
<body>
<header class="bar"><div class="bar-in">
  <div class="brand">
    <svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"
      stroke-linejoin="round" aria-hidden="true"><path d="M20.6 13.4 13.4 20.6a2 2 0 0 1-2.8 0L3 13V3h10l7.6 7.6a2 2 0 0 1 0 2.8Z"/>
      <circle cx="7.5" cy="7.5" r="1.5"/></svg>
    Ценники
  </div>
  <nav class="tabs" id="tabs">
    <button class="tab" data-tab="print">Печать</button>
    <button class="tab" data-tab="products">Товары</button>
    <button class="tab" data-tab="orgs">Организации</button>
  </nav>
</div></header>
<main>
  <section id="print">
    <div class="print-layout">
      <div class="panel">
        <label class="field"><span>Организация</span><select id="p-org"></select></label>
        <div class="field"><span>Сетка на листе A4</span>
          <div class="presets" id="p-presets"></div>
          <div class="grid-inputs">
            <input id="p-cols" type="number" min="1" max="6" aria-label="Колонок"> ×
            <input id="p-rows" type="number" min="1" max="15" aria-label="Рядов">
            <span id="p-cell"></span>
          </div>
          <div class="hint">колонок × рядов, от 1×1 до 6×15</div>
        </div>
        <div class="field"><span>Шрифт наименования</span>
          <div class="scale">
            <input id="p-scale" type="range" min="60" max="200" step="10" aria-label="Размер шрифта наименования, %">
            <output id="p-scale-val"></output>
            <button class="btn link" id="p-scale-reset" title="Вернуть 100%">Сброс</button>
          </div>
        </div>
        <label class="field"><span>Дата на ценнике</span><input id="p-date" type="date"></label>
        <div class="field"><span>Товары</span>
          <div class="picker">
            <input id="p-search" type="search" placeholder="Найти товар и нажать Enter" autocomplete="off">
            <div class="suggest" id="p-suggest" hidden></div>
          </div>
          <ul class="chosen" id="p-chosen"></ul>
          <div class="row-btns">
            <button class="btn" id="p-all">Все товары</button>
            <button class="btn link danger" id="p-clear">Очистить</button>
          </div>
        </div>
        <button class="btn primary print-go" id="p-print">Печать</button>
      </div>
      <div>
        <div class="preview-head"><h1>Предпросмотр</h1><span id="p-summary"></span></div>
        <div class="sheets" id="p-sheets"></div>
      </div>
    </div>
  </section>
  <section id="products"></section>
  <section id="orgs"></section>
</main>
<div id="toast" hidden></div>
<datalist id="units"><option value="шт"><option value="кг"><option value="100 г"><option value="л"><option value="м"><option value="уп"></datalist>
<script>
"use strict";

function el(tag, props, ...children) {
  const e = document.createElement(tag);
  for (const [k, v] of Object.entries(props || {})) {
    if (k === "class") e.className = v;
    else if (k.startsWith("on")) e.addEventListener(k.slice(2), v);
    else if (k in e && k !== "list") e[k] = v;
    else e.setAttribute(k, v);
  }
  for (const c of children) if (c != null) e.append(c);
  return e;
}

let toastTimer = 0;
function toast(text, ok) {
  const t = document.getElementById("toast");
  t.textContent = text;
  t.className = ok ? "ok" : "";
  t.hidden = false;
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => { t.hidden = true; }, ok ? 2000 : 5000);
}

async function api(method, path, body) {
  let res;
  try {
    res = await fetch(path, { method, headers: body ? { "Content-Type": "application/json" } : {},
      body: body ? JSON.stringify(body) : undefined });
  } catch (e) {
    throw new Error("Сервер недоступен");
  }
  if (res.status === 204) return null;
  const data = await res.json().catch(() => null);
  if (!res.ok) throw new Error((data && data.error) || `Ошибка ${res.status}`);
  return data;
}

/* Цена: ввод в рублях ("45,90", "1 200.5"), хранение в копейках без плавающей точки. */
function parsePrice(text) {
  const s = text.replace(/[\s ₽]/g, "").replace(",", ".");
  const m = /^(\d{1,10})(?:\.(\d{1,2}))?$/.exec(s);
  if (!m) throw new Error("Цена: введите число, например 45,90");
  return Number(m[1]) * 100 + Number((m[2] || "0").padEnd(2, "0"));
}
function formatPrice(kop) {
  const rub = Math.floor(kop / 100).toString().replace(/\B(?=(\d{3})+(?!\d))/g, " ");
  return `${rub},${String(kop % 100).padStart(2, "0")}`;
}

/*
 * Таблица справочника с поиском, добавлением и правкой строк на месте.
 * cfg: title, path, empty, addLabel, fields: [{key, label, cls, placeholder, list, text(item), input(item), read(value, item)}]
 */
function catalogTable(root, cfg) {
  let items = [];
  let editing = null;
  const search = el("input", { class: "search", type: "search", placeholder: "Поиск", oninput: render });
  const count = el("span", { class: "count" });
  const add = el("button", { class: "btn primary", textContent: cfg.addLabel, onclick: () => { editing = "new"; render(); } });
  const tbody = el("tbody");
  root.append(
    el("div", { class: "toolbar" }, el("h1", { textContent: cfg.title }), count, el("span", { class: "grow" }), search, add),
    el("div", { class: "card" }, el("table", {},
      el("thead", {}, el("tr", {}, ...cfg.fields.map(f => el("th", { class: f.cls || "", textContent: f.label })), el("th"))),
      tbody)));

  function matches(item, q) {
    return !q || cfg.fields.some(f => f.text(item).toLowerCase().includes(q));
  }

  function viewRow(item) {
    return el("tr", { ondblclick: () => { editing = item.id; render(); } },
      ...cfg.fields.map(f => el("td", { class: f.cls || "", textContent: f.text(item) })),
      el("td", { class: "actions" },
        el("button", { class: "btn link", textContent: "Изменить", onclick: () => { editing = item.id; render(); } }),
        el("button", { class: "btn link danger", textContent: "Удалить", onclick: () => remove(item) })));
  }

  function editRow(item) {
    const inputs = cfg.fields.map(f => el("input", { value: item ? f.input(item) : "", placeholder: f.placeholder || "",
      autocomplete: "off", ...(f.list ? { list: f.list } : {}) }));
    const save = el("button", { class: "btn primary", textContent: "Сохранить" });
    const cancel = el("button", { class: "btn link", textContent: "Отмена", onclick: () => { editing = null; render(); } });
    const row = el("tr", { class: "editing" },
      ...inputs.map((inp, i) => el("td", { class: cfg.fields[i].cls || "" }, inp)),
      el("td", { class: "actions" }, save, cancel));
    async function submit() {
      const body = {};
      inputs.forEach(i => i.classList.remove("bad"));
      for (const [i, f] of cfg.fields.entries()) {
        try {
          Object.assign(body, f.read(inputs[i].value));
        } catch (e) {
          inputs[i].classList.add("bad");
          inputs[i].focus();
          toast(e.message);
          return;
        }
      }
      save.disabled = true;
      try {
        if (item) await api("PUT", `${cfg.path}/${item.id}`, body);
        else await api("POST", cfg.path, body);
        editing = null;
        await load();
        toast("Сохранено", true);
      } catch (e) {
        save.disabled = false;
        toast(e.message);
      }
    }
    save.addEventListener("click", submit);
    row.addEventListener("keydown", e => {
      if (e.key === "Enter") submit();
      if (e.key === "Escape") { editing = null; render(); }
    });
    return row;
  }

  async function remove(item) {
    if (!confirm(`Удалить «${cfg.fields[0].text(item)}»?`)) return;
    try {
      await api("DELETE", `${cfg.path}/${item.id}`);
      await load();
      toast("Удалено", true);
    } catch (e) {
      toast(e.message);
    }
  }

  function render() {
    const q = search.value.trim().toLowerCase();
    const shown = items.filter(i => matches(i, q));
    count.textContent = q ? `${shown.length} из ${items.length}` : `${items.length}`;
    add.disabled = editing === "new";
    const rows = shown.map(i => (i.id === editing ? editRow(i) : viewRow(i)));
    if (editing === "new") rows.unshift(editRow(null));
    if (!rows.length) {
      rows.push(el("tr", {}, el("td", { class: "empty", colSpan: cfg.fields.length + 1,
        textContent: items.length ? "Ничего не найдено" : cfg.empty })));
    }
    tbody.replaceChildren(...rows);
    const first = tbody.querySelector("tr.editing input");
    if (first) first.focus();
  }

  async function load() {
    try {
      items = await api("GET", cfg.path);
      items.sort((a, b) => cfg.fields[0].text(a).localeCompare(cfg.fields[0].text(b), "ru"));
    } catch (e) {
      toast(e.message);
    }
    render();
  }

  load();
  return { load, items: () => items };
}

function required(what) {
  return v => {
    if (!v.trim()) throw new Error(`${what}: заполните поле`);
    return v.trim();
  };
}

const products = catalogTable(document.getElementById("products"), {
  title: "Товары", path: "/api/products", addLabel: "Добавить товар", empty: "Товаров пока нет",
  fields: [
    { key: "name", label: "Наименование", placeholder: "Хлеб белый", text: i => i.name, input: i => i.name,
      read: v => ({ name: required("Наименование")(v) }) },
    { key: "article", label: "Артикул", cls: "muted", text: i => i.article, input: i => i.article,
      read: v => ({ article: v.trim() }) },
    { key: "unit", label: "Ед.", placeholder: "шт", list: "units", text: i => i.unit, input: i => i.unit,
      read: v => ({ unit: required("Единица измерения")(v) }) },
    { key: "price", label: "Цена, ₽", cls: "num", placeholder: "0,00", text: i => formatPrice(i.price_kop),
      input: i => formatPrice(i.price_kop), read: v => ({ price_kop: parsePrice(v) }) },
  ],
});

const orgs = catalogTable(document.getElementById("orgs"), {
  title: "Организации", path: "/api/organizations", addLabel: "Добавить организацию",
  empty: "Организаций пока нет. Название организации печатается в заголовке ценника.",
  fields: [
    { key: "name", label: "Название", placeholder: "ИП Иванов И. И.", text: i => i.name, input: i => i.name,
      read: v => ({ name: required("Название")(v) }) },
  ],
});

/* Печать: выбор организации, товаров и сетки, раскладка ценников по листам A4. */
const printForm = (() => {
  const PRESETS = [[2, 4], [3, 7], [4, 10]];
  const LIMITS = { cols: [1, 6], rows: [1, 15] };
  const $ = id => document.getElementById(id);
  const SCALE = [60, 200, 100];
  const scale = $("p-scale");
  const orgSel = $("p-org"), cols = $("p-cols"), rows = $("p-rows"), date = $("p-date"), search = $("p-search"),
    suggest = $("p-suggest"), chosenList = $("p-chosen"), sheets = $("p-sheets"), summary = $("p-summary");
  let allProducts = [], allOrgs = [], chosen = [], cursor = 0;

  const store = {
    get() { try { return JSON.parse(localStorage.getItem("print") || "{}"); } catch (e) { return {}; } },
    set(v) { try { localStorage.setItem("print", JSON.stringify(v)); } catch (e) { /* хранилище недоступно */ } },
  };
  function save() {
    store.set({ org: orgSel.value, cols: cols.value, rows: rows.value, scale: scale.value, products: chosen });
  }

  const clamp = (v, [lo, hi], def) => { const n = parseInt(v, 10); return Number.isFinite(n) ? Math.min(hi, Math.max(lo, n)) : def; };
  const grid = () => [clamp(cols.value, LIMITS.cols, 3), clamp(rows.value, LIMITS.rows, 7)];
  const today = () => { const d = new Date(); d.setMinutes(d.getMinutes() - d.getTimezoneOffset()); return d.toISOString().slice(0, 10); };
  const byId = id => allProducts.find(p => p.id === id);

  const presetBox = $("p-presets");
  for (const [c, r] of PRESETS) {
    presetBox.append(el("button", { class: "btn", textContent: `${c}×${r}`,
      onclick: () => { cols.value = c; rows.value = r; update(); } }));
  }

  function tag(p, org, dateText) {
    const rub = Math.floor(p.price_kop / 100).toString().replace(/\B(?=(\d{3})+(?!\d))/g, " ");
    const kop = String(p.price_kop % 100).padStart(2, "0");
    return el("div", { class: "tag" }, el("div", { class: "tag-in" },
      el("div", { class: "tag-org", textContent: org || " " }),
      el("div", { class: "tag-name-box" }, el("div", { class: "tag-name", textContent: p.name, title: p.name })),
      el("div", { class: "tag-row" }, el("span", { class: "tag-unit", textContent: `за ${p.unit}` }),
        el("div", { class: "tag-price", style: `--len: ${Math.max(2.6, rub.replace(/\u202f/g, "").length + 0.3 * (rub.match(/\u202f/g) || []).length)}` }, el("span", { class: "rub", textContent: rub }), el("span", { class: "kop", textContent: kop }),
          el("span", { class: "cur", textContent: "₽" }))),
      el("div", { class: "tag-foot" }, el("span", { textContent: p.article ? `${dateText} · Арт. ${p.article}` : dateText }),
        el("span", { class: "sign" }, "Подпись", el("i")))));
  }

  function renderSheets() {
    const [c, r] = grid();
    const per = c * r;
    const org = (allOrgs.find(o => String(o.id) === orgSel.value) || {}).name || "";
    const dateText = date.value ? date.value.split("-").reverse().join(".") : "";
    const items = chosen.map(byId).filter(Boolean);
    const pages = [];
    for (let i = 0; i < items.length; i += per) {
      const sheet = el("div", { class: "sheet" }, ...items.slice(i, i + per).map(p => tag(p, org, dateText)));
      sheet.style.setProperty("--cols", c);
      sheet.style.setProperty("--rows", r);
      pages.push(sheet);
    }
    sheets.style.setProperty("--name-scale", clamp(scale.value, SCALE, SCALE[2]) / 100);
    sheets.replaceChildren(...pages);
    fitNames();
    const n = pages.length;
    summary.textContent = items.length ? `${items.length} шт., ${n} ${n === 1 ? "лист" : n < 5 ? "листа" : "листов"}` : "добавьте товары слева";
    $("p-print").disabled = !items.length;
  }

  /* Наименование показывает столько строк, сколько помещается между шапкой и ценой (до 4). */
  function fitNames() {
    for (const name of sheets.querySelectorAll(".tag-name")) {
      const box = name.parentElement.clientHeight;
      if (!box) continue;
      const lh = parseFloat(getComputedStyle(name).lineHeight);
      name.style.webkitLineClamp = String(Math.max(1, Math.min(4, Math.floor(box / lh))));
    }
  }

  function renderChosen() {
    const items = chosen.map(byId).filter(Boolean);
    chosenList.replaceChildren(...(items.length ? items.map(p => el("li", {},
      el("span", { class: "nm", textContent: p.name, title: p.name }),
      el("span", { class: "price", textContent: formatPrice(p.price_kop) }),
      el("button", { class: "btn link danger", textContent: "×", title: "Убрать",
        onclick: () => { chosen = chosen.filter(id => id !== p.id); update(); } })))
      : [el("li", { class: "empty hint", textContent: "Список пуст" })]));
  }

  function update() {
    const [c, r] = grid();
    for (const b of presetBox.children) b.classList.toggle("active", b.textContent === `${c}×${r}`);
    $("p-cell").textContent = `ячейка ${((194 / c)).toFixed(0)}×${((280 / r)).toFixed(0)} мм`;
    renderChosen();
    renderSheets();
    save();
  }

  function candidates() {
    const q = search.value.trim().toLowerCase();
    if (!q) return [];
    return allProducts.filter(p => !chosen.includes(p.id) &&
      (p.name.toLowerCase().includes(q) || p.article.toLowerCase().includes(q))).slice(0, 30);
  }
  function renderSuggest() {
    const list = candidates();
    cursor = Math.min(cursor, Math.max(0, list.length - 1));
    suggest.hidden = !search.value.trim();
    suggest.replaceChildren(...(list.length ? list.map((p, i) => el("button", { class: i === cursor ? "cur" : "",
      onmousedown: e => { e.preventDefault(); pick(p); } },
      el("span", { textContent: p.name }), el("span", { class: "price", textContent: formatPrice(p.price_kop) })))
      : [el("div", { class: "none", textContent: "Не найдено" })]));
  }
  function pick(p) {
    chosen.push(p.id);
    search.value = "";
    cursor = 0;
    renderSuggest();
    update();
    search.focus();
  }
  search.addEventListener("input", () => { cursor = 0; renderSuggest(); });
  search.addEventListener("blur", () => { suggest.hidden = true; });
  search.addEventListener("focus", renderSuggest);
  search.addEventListener("keydown", e => {
    const list = candidates();
    if (e.key === "ArrowDown") { cursor = Math.min(cursor + 1, list.length - 1); renderSuggest(); e.preventDefault(); }
    else if (e.key === "ArrowUp") { cursor = Math.max(cursor - 1, 0); renderSuggest(); e.preventDefault(); }
    else if (e.key === "Enter" && list[cursor]) { pick(list[cursor]); e.preventDefault(); }
    else if (e.key === "Escape") { search.value = ""; renderSuggest(); }
  });

  for (const inp of [cols, rows]) {
    inp.addEventListener("input", update);
    inp.addEventListener("change", () => { const [c, r] = grid(); cols.value = c; rows.value = r; update(); });
  }
  orgSel.addEventListener("change", update);
  const showScale = () => { $("p-scale-val").textContent = `${scale.value}%`; };
  scale.addEventListener("input", () => { showScale(); renderSheets(); save(); });
  $("p-scale-reset").addEventListener("click", () => { scale.value = SCALE[2]; showScale(); renderSheets(); save(); });
  date.addEventListener("change", renderSheets);
  $("p-all").addEventListener("click", () => {
    const have = new Set(chosen);
    chosen.push(...allProducts.map(p => p.id).filter(id => !have.has(id)));
    update();
  });
  $("p-clear").addEventListener("click", () => { chosen = []; update(); });
  $("p-print").addEventListener("click", () => window.print());
  window.addEventListener("beforeprint", fitNames);
  window.addEventListener("afterprint", fitNames);

  const saved = store.get();
  cols.value = clamp(saved.cols, LIMITS.cols, 3);
  rows.value = clamp(saved.rows, LIMITS.rows, 7);
  scale.value = clamp(saved.scale, SCALE, SCALE[2]);
  showScale();
  chosen = Array.isArray(saved.products) ? saved.products.filter(Number.isInteger) : [];
  date.value = today();

  async function load() {
    try {
      [allProducts, allOrgs] = await Promise.all([api("GET", "/api/products"), api("GET", "/api/organizations")]);
    } catch (e) {
      toast(e.message);
    }
    allOrgs.sort((a, b) => a.name.localeCompare(b.name, "ru"));
    allProducts.sort((a, b) => a.name.localeCompare(b.name, "ru"));
    const want = orgSel.value || store.get().org || "";
    orgSel.replaceChildren(el("option", { value: "", textContent: "— без организации —" }),
      ...allOrgs.map(o => el("option", { value: String(o.id), textContent: o.name })));
    orgSel.value = allOrgs.some(o => String(o.id) === want) ? want : (allOrgs[0] ? String(allOrgs[0].id) : "");
    chosen = chosen.filter(id => byId(id));
    update();
  }
  return { load };
})();

function showTab() {
  const names = [...document.querySelectorAll("#tabs .tab")].map(t => t.dataset.tab);
  const cur = names.includes(location.hash.slice(1)) ? location.hash.slice(1) : names[0];
  for (const t of document.querySelectorAll("#tabs .tab")) t.classList.toggle("active", t.dataset.tab === cur);
  for (const n of names) document.getElementById(n).hidden = n !== cur;
  if (cur === "print") printForm.load();
}
document.getElementById("tabs").addEventListener("click", e => {
  const t = e.target.closest(".tab");
  if (t) location.hash = t.dataset.tab;
});
window.addEventListener("hashchange", showTab);
showTab();
</script>
</body>
</html>
)HTML";

} // namespace

std::string_view index_page() { return PAGE; }
