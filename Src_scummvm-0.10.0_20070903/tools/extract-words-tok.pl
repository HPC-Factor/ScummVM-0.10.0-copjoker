#!perl
#
# ScummVM - Scumm Interpreter
# Copyright (C) 2006 The ScummVM project
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/tags/release-0-10-0/tools/extract-words-tok.pl $
# $Id: extract-words-tok.pl 24635 2006-11-06 13:19:12Z sev $
#

# This script extracts AGI words.tok file
#
# It produces one word per line. Multiword verbs get splitted
#
# Typical usage:
#
# for i in agigames/*/words.tok
# do
#   tools/extract-words-tok.pl "$i"
# done | tools/construct-pred-dict.pl
#

local $/;
local $file = <>;

#$off = ord(substr($file, $i * 2, 1)) * 256 + ord(substr($file, $i * 2 + 1, 1));
#$offn = ord(substr($file, ($i + 1) * 2, 1)) * 256 + ord(substr($file, ($i + 1) * 2 + 1, 1));

$off = 52;

$word = "";
$mode = 0;

while ($off < length $file) {
  $c = (ord(substr($file, $off, 1)));
  if ($mode == 0) {
    $word = substr $word, 0, $c;
    $mode = 1;
    $off++;
    next;
  }

  $r = ($c & 0x7f) ^ 0x7f;
  $word .= chr($r);

  if ($c & 0x80) {
    for $w (split ' ', $word) {
      print "$w\n";
    }
    $off += 3;
    $mode = 0;
    next;
  }

  $off++;
}

for $w (split ' ', $word) {
  print "$w\n";
}
