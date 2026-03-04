# Maintainer: Nick <nick@example.com>
pkgname=cybergrind-git
pkgver=r5.1.0.0
pkgrel=1
pkgdesc="A terminal-based idle game with a cyberpunk theme"
arch=('x86_64')
url="https://github.com/nick/whiletrue" # Keep repo URL as whiletrue
license=('Apache')
depends=('ncurses')
makedepends=('git' 'gcc' 'make')
provides=('cybergrind')
conflicts=('cybergrind')
source=('whiletrue::git+https://github.com/yourusername/whiletrue.git')
md5sums=('SKIP')

pkgver() {
  cd "$srcdir/whiletrue"
  printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

build() {
  cd "$srcdir/whiletrue"
  make
}

package() {
  cd "$srcdir/whiletrue"
  make DESTDIR="$pkgdir" PREFIX=/usr install
}
