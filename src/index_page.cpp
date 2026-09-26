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
    <button class="tab" data-tab="products">Товары</button>
    <button class="tab" data-tab="orgs">Организации</button>
  </nav>
</div></header>
<main>
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

function showTab() {
  const names = [...document.querySelectorAll("#tabs .tab")].map(t => t.dataset.tab);
  const cur = names.includes(location.hash.slice(1)) ? location.hash.slice(1) : names[0];
  for (const t of document.querySelectorAll("#tabs .tab")) t.classList.toggle("active", t.dataset.tab === cur);
  for (const n of names) document.getElementById(n).hidden = n !== cur;
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
