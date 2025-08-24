#!/bin/bash
set -e

echo "Установка зависимостей для сборки GTK3 и GtkSourceView..."

sudo apt update
sudo apt install -y build-essential pkg-config libgtk-3-dev libgtksourceview-4-dev

echo "Зависимости установлены."
