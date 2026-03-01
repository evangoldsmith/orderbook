#!/usr/bin/env python3
"""Live terminal monitor for orderbook.log — run alongside the demo."""

import re
import time
import sys
import os
from pathlib import Path
from collections import deque

try:
    from rich.live import Live
    from rich.layout import Layout
    from rich.panel import Panel
    from rich.table import Table
    from rich.text import Text
    from rich.console import Console
    from rich.align import Align
    from rich import box as rbox
except ImportError:
    print("Missing dependency:  pip install rich")
    sys.exit(1)

LOG_PATH = Path("logs/orderbook.log")
REFRESH_HZ = 15
MAX_EVENTS = 30
MAX_TRADE_HISTORY = 80

ORDER_RE  = re.compile(r'\((\d+)\)\[ORDER\]: \{Side: (\w+), qty: (\d+), price: ([\d.]+), id: (\d+)\}')
TRADE_RE  = re.compile(r'\((\d+)\)\[TRADE\]: \{qty: (\d+), price: ([\d.]+), buyerId: (\d+), sellerId: (\d+)\}')
CANCEL_RE = re.compile(r'\((\d+)\)\[CANCEL\]: \{id: (\d+), price: ([\d.]+), qty: (\d+)\}')


class Stats:
    def __init__(self):
        self.total_orders  = 0
        self.buy_orders    = 0
        self.sell_orders   = 0
        self.total_trades  = 0
        self.total_cancels = 0
        self.volume        = 0
        self.notional      = 0.0
        self.last_price    = None
        self.high          = None
        self.low           = None
        self.trade_prices  = deque(maxlen=MAX_TRADE_HISTORY)
        self.trade_times   = deque(maxlen=MAX_TRADE_HISTORY)  # ns timestamps
        self.events        = deque(maxlen=MAX_EVENTS)
        self.start_ns      = None

    @property
    def vwap(self):
        return self.notional / self.volume if self.volume else None

    @property
    def trades_per_min(self):
        if len(self.trade_times) < 2:
            return 0.0
        span_ns = self.trade_times[-1] - self.trade_times[0]
        if span_ns <= 0:
            return 0.0
        return len(self.trade_times) / (span_ns / 60e9)

    def process_line(self, line):
        line = line.strip()
        if not line:
            return

        m = TRADE_RE.match(line)
        if m:
            ts, qty, price = int(m.group(1)), int(m.group(2)), float(m.group(3))
            buyer, seller  = m.group(4), m.group(5)
            if self.start_ns is None:
                self.start_ns = ts
            self.total_trades += 1
            self.volume       += qty
            self.notional     += qty * price
            self.last_price    = price
            self.high = price if self.high is None else max(self.high, price)
            self.low  = price if self.low  is None else min(self.low,  price)
            self.trade_prices.append(price)
            self.trade_times.append(ts)
            t = Text()
            t.append("✓ TRADE  ", style="bold yellow")
            t.append(f"qty={qty:<3} @ ", style="white")
            t.append(f"${price:.2f}", style="bold yellow")
            t.append(f"  b={buyer} s={seller}", style="dim")
            self.events.appendleft(t)
            return

        m = ORDER_RE.match(line)
        if m:
            _, side, qty, price, oid = m.group(1), m.group(2), int(m.group(3)), float(m.group(4)), m.group(5)
            self.total_orders += 1
            if side == "BUY":
                self.buy_orders += 1
                style, arrow = "green", "↑ BUY   "
            else:
                self.sell_orders += 1
                style, arrow = "red", "↓ SELL  "
            t = Text()
            t.append(arrow, style=f"bold {style}")
            t.append(f"qty={qty:<3} @ ", style="white")
            t.append(f"${price:.2f}", style=style)
            t.append(f"  id={oid}", style="dim")
            self.events.appendleft(t)
            return

        m = CANCEL_RE.match(line)
        if m:
            _, oid, price, qty = m.group(1), m.group(2), float(m.group(3)), int(m.group(4))
            self.total_cancels += 1
            t = Text()
            t.append("✕ CANCEL ", style="bold magenta")
            t.append(f"id={oid}", style="magenta")
            t.append(f"  qty={qty:<3} @ ${price:.2f}", style="dim")
            self.events.appendleft(t)


def sparkline(prices, width=68, height=7):
    """ASCII dot chart of trade prices."""
    if len(prices) < 2:
        return Align.center(Text("Waiting for trades…", style="dim"), vertical="middle")

    pts   = list(prices)[-width:]
    lo    = min(pts)
    hi    = max(pts)
    span  = hi - lo if hi != lo else 1.0

    grid = [[" "] * len(pts) for _ in range(height)]
    for col, p in enumerate(pts):
        row = round((1.0 - (p - lo) / span) * (height - 1))
        grid[row][col] = "●"

    lines = Text()
    for r, row in enumerate(grid):
        # price label on the right edge
        frac  = 1.0 - r / (height - 1)
        label = f" {lo + frac * span:>7.2f}"
        if r == 0:
            style = "bright_red"
        elif r == height - 1:
            style = "bright_green"
        else:
            style = "dim"

        row_text = "".join(row)
        # colour the dots: green below midpoint, red above
        mid_row = (height - 1) / 2
        dot_style = "bright_green" if r > mid_row else "bright_red"
        colored = row_text.replace("●", "●")  # placeholder; build manually
        lines.append("  ")
        for ch in row_text:
            if ch == "●":
                lines.append("●", style=dot_style)
            else:
                lines.append(ch)
        lines.append(label, style=style)
        lines.append("\n")

    return lines


def build_ui(stats: Stats) -> Layout:
    layout = Layout()
    layout.split_column(
        Layout(name="header", size=3),
        Layout(name="body"),
        Layout(name="chart", size=11),
    )
    layout["body"].split_row(
        Layout(name="stats", ratio=1),
        Layout(name="feed", ratio=2),
    )

    # ── Header ────────────────────────────────────────────────────────────────
    header_text = Text("  LIVE ORDERBOOK MONITOR  ", style="bold black on bold cyan", justify="center")
    layout["header"].update(Panel(Align.center(header_text, vertical="middle"), style="bold cyan"))

    # ── Stats ─────────────────────────────────────────────────────────────────
    tbl = Table(box=None, padding=(0, 2), show_header=False, expand=True)
    tbl.add_column(style="dim",  no_wrap=True)
    tbl.add_column(justify="right", style="bold white", no_wrap=True)

    last_str  = f"${stats.last_price:.2f}" if stats.last_price else "—"
    vwap_str  = f"${stats.vwap:.2f}"       if stats.vwap       else "—"
    high_str  = f"${stats.high:.2f}"       if stats.high       else "—"
    low_str   = f"${stats.low:.2f}"        if stats.low        else "—"
    tpm_str   = f"{stats.trades_per_min:.1f}/min"

    tbl.add_row("Orders",    str(stats.total_orders))
    tbl.add_row("  ↑ Buys",  f"[green]{stats.buy_orders}[/green]")
    tbl.add_row("  ↓ Sells", f"[red]{stats.sell_orders}[/red]")
    tbl.add_row("Trades",    f"[yellow]{stats.total_trades}[/yellow]")
    tbl.add_row("Cancels",   f"[magenta]{stats.total_cancels}[/magenta]")
    tbl.add_row("", "")
    tbl.add_row("Volume",    str(stats.volume))
    tbl.add_row("VWAP",      vwap_str)
    tbl.add_row("Last",      last_str)
    tbl.add_row("High",      f"[bright_red]{high_str}[/bright_red]")
    tbl.add_row("Low",       f"[bright_green]{low_str}[/bright_green]")
    tbl.add_row("", "")
    tbl.add_row("Trade rate", tpm_str)

    layout["stats"].update(Panel(tbl, title="[bold cyan]Stats[/bold cyan]", border_style="cyan"))

    # ── Event feed ────────────────────────────────────────────────────────────
    feed = Table(box=None, padding=(0, 0), show_header=False, expand=True)
    feed.add_column(no_wrap=True)
    for event in stats.events:
        feed.add_row(event)

    layout["feed"].update(Panel(feed, title="[bold cyan]Events[/bold cyan]", border_style="cyan"))

    # ── Chart ─────────────────────────────────────────────────────────────────
    layout["chart"].update(
        Panel(sparkline(stats.trade_prices), title="[bold cyan]Trade Price History[/bold cyan]", border_style="cyan")
    )

    return layout


def tail(f, stats: Stats):
    """Read new lines from f, return True if any were processed."""
    found = False
    while True:
        line = f.readline()
        if not line:
            break
        stats.process_line(line)
        found = True
    return found


def main():
    console = Console()

    console.print("[dim]Waiting for logs/orderbook.log …  (start the demo)[/dim]")
    while not LOG_PATH.exists():
        time.sleep(0.2)

    stats   = Stats()
    last_inode = LOG_PATH.stat().st_ino

    with open(LOG_PATH, "r") as f:
        # Consume any existing content before going live
        for line in f:
            stats.process_line(line)

        with Live(build_ui(stats), refresh_per_second=REFRESH_HZ, screen=True) as live:
            try:
                while True:
                    # Detect log rotation / truncation (demo reopens with trunc)
                    try:
                        cur_inode = LOG_PATH.stat().st_ino
                        cur_size  = LOG_PATH.stat().st_size
                    except FileNotFoundError:
                        time.sleep(0.1)
                        continue

                    if cur_inode != last_inode or cur_size < f.tell():
                        f.close()
                        f = open(LOG_PATH, "r")
                        stats   = Stats()
                        last_inode = cur_inode

                    if tail(f, stats):
                        live.update(build_ui(stats))
                    else:
                        time.sleep(1 / REFRESH_HZ)

            except KeyboardInterrupt:
                pass

    console.print("\n[bold]Monitor stopped.[/bold]")


if __name__ == "__main__":
    main()
