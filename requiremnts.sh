#!/bin/bash
set -e

echo "Установка зависимостей для сборки GTK3 и GtkSourceView..."

sudo apt update
sudo apt install -y build-essential pkg-config libgtk-3-dev libgtksourceview-4-dev

echo "Зависимости установлены."


set -e

echo "Установка зависимостей для сборки терминального редактора с ncurses..."

sudo apt update
sudo apt install -y build-essential pkg-config libncurses-dev

echo "Зависимости установлены."



sudo apt update
sudo apt install liblua5.3-dev
