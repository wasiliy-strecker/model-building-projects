"""Sichtbarer X11-Ablauf mit eigenen Fenstern und ausschließlich synthetischen Daten."""

import argparse
import contextlib
import fcntl
import json
import os
import re
import sqlite3
import subprocess
import tempfile
import time
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--clean-environment", action="store_true")
    args = parser.parse_args()
    # Shared visible desktop is serialized. A separately owned X server may run in parallel.
    with contextlib.ExitStack() as stack:
        if os.environ.get("DISPLAY", "").split(".")[0] in (":0", ":0.0"):
            lock = stack.enter_context(
                open("/tmp/app-factory-cpp-visible-ui.lock", "a")
            )
            fcntl.flock(lock, fcntl.LOCK_EX)
        run(args)


def run(args):
    binary = args.binary.resolve()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ)
    if args.clean_environment:
        for key in list(env):
            if key.startswith(("QT_", "QML_", "MODELLBAUPROJEKT_")) or key in (
                "LD_LIBRARY_PATH",
                "CMAKE_PREFIX_PATH",
            ):
                env.pop(key)
        env["PATH"] = "/usr/bin:/bin"
    env.update(QT_QPA_PLATFORM="xcb", QT_QPA_PLATFORMTHEME="gtk3", QT_SCALE_FACTOR="1")
    process = None
    active = None

    def command(*args):
        return subprocess.check_output(
            args, env=env, text=True, stderr=subprocess.PIPE, timeout=6
        ).strip()

    has_wm = "window id #" in command("xprop", "-root", "_NET_SUPPORTING_WM_CHECK")
    if not has_wm:
        raise SystemExit(
            "Für native GTK-Dialoge bitte einen X11-Desktop mit Fenstermanager verwenden."
        )

    def wait(predicate, timeout=8):
        end = time.monotonic() + timeout
        while time.monotonic() < end:
            try:
                result = predicate()
                if result:
                    return result
            except (subprocess.CalledProcessError, sqlite3.OperationalError):
                pass
            time.sleep(0.05)
        raise AssertionError("GUI-Zustand nicht erreicht")

    def geometry(identifier):
        return dict(
            line.split("=", 1)
            for line in command(
                "xdotool", "getwindowgeometry", "--shell", identifier
            ).splitlines()
        )

    def window(title):
        nonlocal active

        def find():
            identifiers = command(
                "xdotool",
                "search",
                "--all",
                "--onlyvisible",
                "--pid",
                str(process.pid),
                "--name",
                title,
            ).splitlines()
            candidates = []
            for identifier in identifiers:
                if command("xdotool", "getwindowpid", identifier) != str(process.pid):
                    continue
                shape = geometry(identifier)
                area = int(shape["WIDTH"]) * int(shape["HEIGHT"])
                if area > 20000:
                    candidates.append((area, identifier))
            return max(candidates)[1] if candidates else None

        active = wait(find)
        if has_wm:
            command("xdotool", "windowactivate", "--sync", active)
        command("xdotool", "windowraise", active, "windowfocus", "--sync", active)
        time.sleep(0.2)
        return active

    def focus_owned():
        assert command("xdotool", "getwindowpid", active) == str(process.pid)
        if has_wm:
            command("xdotool", "windowactivate", "--sync", active)
        command("xdotool", "windowfocus", "--sync", active)
        assert command("xdotool", "getwindowfocus") == active

    def key(*keys):
        for value in keys:
            focus_owned()
            command("xdotool", "key", "--clearmodifiers", value)
            time.sleep(0.2)

    def save():
        shape = geometry(active)
        click(active, int(shape["WIDTH"]) - 65, int(shape["HEIGHT"]) - 34)

    def add_project():
        key("ctrl+n")

    def text(value):
        focus_owned()
        command(
            "xdotool",
            "type",
            "--clearmodifiers",
            "--delay",
            "2",
            value,
        )
        time.sleep(0.12)

    def capture(identifier, name):
        time.sleep(0.3)
        command("import", "-window", identifier, str(output / (name + ".png")))

    def click(identifier, x, y):
        nonlocal active
        active = identifier
        focus_owned()
        info = command("xwininfo", "-id", identifier)
        origin_x = int(re.search(r"Absolute upper-left X:\s*(-?\d+)", info)[1])
        origin_y = int(re.search(r"Absolute upper-left Y:\s*(-?\d+)", info)[1])
        command(
            "xdotool",
            "mousemove",
            str(origin_x + x),
            str(origin_y + y),
            "click",
            "1",
        )
        time.sleep(0.2)

    def destructive(identifier, discard=False):
        shape = geometry(identifier)
        click(
            identifier,
            int(shape["WIDTH"]) - (198 if discard else 140),
            int(shape["HEIGHT"]) - 24,
        )

    with tempfile.TemporaryDirectory(prefix="modellbauprojekt-gui-") as tmp:
        directory = Path(tmp)
        (directory / "language.txt").write_text("de-DE\n")
        database = directory / "modellbauprojekt.sqlite3"
        source = directory / "synthetischer-gleiter.png"
        replacement = directory / "synthetisches-boot.jpg"
        svg = directory / "synthetisches-modell.svg"
        svg.write_text(
            """<svg xmlns="http://www.w3.org/2000/svg" width="900" height="600" viewBox="0 0 900 600"><rect width="900" height="600" fill="#e4ddd0"/><rect x="30" y="30" width="840" height="540" rx="24" fill="#234b54"/><path d="M90 100H810M90 200H810M90 300H810M90 400H810M90 500H810M150 80V520M300 80V520M450 80V520M600 80V520M750 80V520" stroke="#38636d" stroke-width="2"/><path d="M150 355L394 285L412 130L455 115L487 285L750 355L748 390L485 340L480 440L540 475V495L450 472L360 495V475L415 440L413 340L152 390Z" fill="#e7d6b3"/><circle cx="450" cy="296" r="17" fill="#d18d51"/></svg>"""
        )
        subprocess.run(["convert", str(svg), str(source)], env=env, check=True)
        subprocess.run(
            [
                "convert",
                str(source),
                "-flop",
                "-modulate",
                "100,80,115",
                str(replacement),
            ],
            env=env,
            check=True,
        )
        source_bytes = source.read_bytes()
        replacement_bytes = replacement.read_bytes()
        log = (output / "app.log").open("w")

        def rows():
            if not database.exists():
                return None
            with sqlite3.connect(
                f"file:{database}?mode=ro", uri=True, timeout=0.1
            ) as db:
                return db.execute(
                    "SELECT id,name,manufacturer,location,note,status,photo FROM projects ORDER BY id"
                ).fetchall()

        def start():
            nonlocal process
            process = subprocess.Popen(
                [str(binary), "--data-dir", str(directory)],
                cwd=directory,
                env=env,
                stdout=log,
                stderr=log,
            )
            main_window = window("^Model Building Projects$")
            wait(lambda: rows() is not None)
            time.sleep(0.35)
            return main_window

        def stop():
            window("^Model Building Projects$")
            key("alt+F4")
            process.wait(timeout=8)
            assert process.returncode == 0

        def editor():
            window("^Model Building Projects$")
            time.sleep(1.0)
            key("alt+b")
            return window("^Projekt bearbeiten$")

        def import_file(path, screenshot=None):
            key("alt+f")
            picker = window("^Projektfoto auswählen$")
            key("ctrl+l")
            text(str(path))
            if screenshot:
                capture(picker, screenshot)
            key("Return")
            window("^Projekt (hinzufügen|bearbeiten)$")
            time.sleep(0.5)

        try:
            main_window = start()
            assert rows() == []
            capture(main_window, "01-leer")
            add_project()
            edit_window = window("^Projekt hinzufügen$")
            save()
            capture(edit_window, "02-pflichtfeld")
            key("alt+p")
            text("Segelflugzeug Möwe")
            key("alt+b")
            text("Musterbausatz 1:48")
            key("alt+a")
            text("Werkzimmer, Regal 2")
            key("alt+n")
            text("  Flügel zunächst trocken anpassen.")
            key("Return")
            text("Blauer Streifen an der Tragfläche.")
            key("alt+s", "Home", "Down")
            # Cancel the native file chooser before a successful import.
            key("alt+f")
            window("^Projektfoto auswählen$")
            key("Escape")
            window("^Projekt hinzufügen$")
            import_file(source, "03-native-dateiauswahl")
            capture(edit_window, "04-gemeinsamer-entwurf")
            save()
            wait(lambda: len(rows()) == 1)
            original = rows()[0]
            assert original[1:] == (
                "Segelflugzeug Möwe",
                "Musterbausatz 1:48",
                "Werkzimmer, Regal 2",
                "  Flügel zunächst trocken anpassen.\nBlauer Streifen an der Tragfläche.",
                "started",
                source_bytes,
            ), original[:6]
            window("^Model Building Projects$")
            capture(main_window, "05-gespeichert")
            key("ctrl+f")
            text("TRAGFLÄCHE")
            capture(main_window, "06-notizsuche")
            key("ctrl+a")
            text("nicht-vorhanden")
            capture(main_window, "07-ohne-treffer")
            key("ctrl+a", "BackSpace")
            edit_window = editor()
            key("alt+s", "End", "Escape")
            confirmation = window("Änderungen verwerfen")
            capture(confirmation, "08-status-verwerfschutz")
            key("Return")
            window("^Projekt bearbeiten$")
            key("Escape")
            confirmation = window("Änderungen verwerfen")
            destructive(confirmation, discard=True)
            assert rows()[0] == original
            edit_window = editor()
            key("alt+p", "ctrl+a")
            text("Segelflugzeug Möwe – lackiert")
            key("alt+s", "End")
            import_file(replacement)
            with sqlite3.connect(database) as blocker:
                blocker.execute("BEGIN EXCLUSIVE")
                save()
                time.sleep(2.3)
                window("^Projekt bearbeiten$")
                capture(edit_window, "09-speicherfehler")
                command("xdotool", "windowsize", edit_window, "400", "360")
                capture(edit_window, "10-kleiner-fehlerdialog")
                blocker.rollback()
            assert rows()[0] == original
            save()
            wait(lambda: rows()[0][5] == "finished")
            assert rows()[0][6] == replacement_bytes
            assert rows()[0][0] == original[0]
            assert rows()[0][4] == original[4]
            editor()
            key("alt+s", "Home")
            save()
            wait(lambda: rows()[0][5] == "planned")
            assert rows()[0][6] == replacement_bytes
            editor()
            key("alt+s", "Down")
            save()
            wait(lambda: rows()[0][5] == "started")
            window("^Model Building Projects$")
            add_project()
            edit_window = window("^Projekt hinzufügen$")
            key("alt+p")
            text("Segelflugzeug Möwe – lackiert")
            key("alt+s", "End")
            import_file(source)
            save()
            wait(lambda: len(rows()) == 2)
            assert rows()[1][5:] == ("finished", source_bytes)
            assert rows()[0][0] != rows()[1][0]
            window("^Model Building Projects$")
            capture(main_window, "11-gleiche-namen")
            source.unlink()
            replacement.unlink()
            before = rows()
            stop()
            main_window = start()
            assert rows() == before
            capture(main_window, "12-neustart-ohne-bildquelle")
            maps = Path(f"/proc/{process.pid}/maps").read_text()
            libraries = sorted(
                {
                    line.split()[-1]
                    for line in maps.splitlines()
                    if any(
                        part in line
                        for part in (
                            "libQt6",
                            "libqsqlite",
                            "libqjpeg",
                            "libqgtk3",
                            "libicu",
                        )
                    )
                }
            )
            (output / "loaded-libraries.json").write_text(
                json.dumps(libraries, indent=2) + "\n"
            )
            if args.clean_environment:
                assert all(str(binary.parent) in path for path in libraries), libraries
                assert any("libqsqlite" in path for path in libraries)
                assert any("libqjpeg" in path for path in libraries)
                assert any("libqgtk3" in path for path in libraries)
            editor()
            key("alt+e")
            save()
            wait(lambda: rows()[0][6] is None)
            assert rows()[0][5] == "started"
            assert rows()[1][6] == source_bytes
            window("^Model Building Projects$")
            key("alt+l")
            window("Projekt löschen")
            key("Escape")
            assert len(rows()) == 2
            window("^Model Building Projects$")
            key("alt+l")
            confirmation = window("Projekt löschen")
            capture(confirmation, "13-loeschbestaetigung")
            destructive(confirmation)
            wait(lambda: len(rows()) == 1)
            assert rows()[0][0] == before[1][0]
            window("^Model Building Projects$")
            key("alt+l")
            destructive(window("Projekt löschen"))
            wait(lambda: rows() == [])
            window("^Model Building Projects$")
            capture(main_window, "14-wieder-leer")
            stop()
            (output / "result.json").write_text(
                json.dumps(
                    {
                        "result": "passed",
                        "binary": str(binary),
                        "clean_environment": args.clean_environment,
                        "native_picker": "GTK 3",
                        "synthetic_data_only": True,
                        "screenshots": 14,
                    },
                    indent=2,
                )
                + "\n"
            )
            print(f"Sichtbare Prüfung bestanden: {output}")
        finally:
            if process is not None and process.poll() is None:
                process.terminate()
                process.wait(timeout=5)
            log.close()


if __name__ == "__main__":
    main()
