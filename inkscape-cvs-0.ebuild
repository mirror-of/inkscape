# Copyright 1999-2003 Gentoo Technologies, Inc.
# Distributed under the terms of the GNU General Public License v2
# $Header$

# Inkscape CVS ebuild
# Just stick this file in an appropriate place under a portage tree, such as
# media-gfx/inkscape-cvs and run 'ebuild inkscape-cvs-0.ebuild digest'.  Then
# 'emerge inkscape-cvs'.  Keep in mind this builds inkscape from cvs HEAD, so
# it could work fine one minute and break the next.

inherit cvs

ECVS_SERVER="cvs.sourceforge.net:/cvsroot/inkscape"
ECVS_MODULE="inkscape"

S=${WORKDIR}/${ECVS_MODULE}

DESCRIPTION="Inkscape is an open source SVG editor with capabilities similar to Illustrator, CorelDraw, Visio, etc."
HOMEPAGE="http://www.inkscape.org/"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~x86"

# Other deps?
DEPEND="dev-libs/libsigc++"

RDEPEND=${DEPEND}

src_compile() {
    myconf="--program-suffix=-cvs ${myconf}"
    ./autogen.sh ${myconf} || die
    econf ${myconf} || die "Configure failed"
    emake || die "Make failed"
}

src_install() {
    dodoc AUTHORS COPYING HACKING ChangeLog INSTALL NEWS README TODO
    make DESTDIR="${D}" install || die "Install failed"
}
