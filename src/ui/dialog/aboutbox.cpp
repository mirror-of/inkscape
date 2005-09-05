/**
 * \brief HandleBox Widget - Adds a detachment handle to another widget.
 *
 * This work really doesn't amount to much more than a convenience constructor
 * for Gtk::HandleBox.  Maybe this could be contributed back to Gtkmm, as
 * Gtkmm provides several convenience constructors for other widgets as well.
 *
 * Author:
 *   Derek P. Moore <derekm@hackunix.org>
 *
 * Copyright (C) 2004 Derek P. Moore
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glibmm/i18n.h>
#include "aboutbox.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

LicenseBox::LicenseBox(Gtk::Window& parent, Glib::ustring& license)
    : AboutBoxChild(parent, _("License"))
{
    // This attempts to emulate the AboutDialog License dialog settings as
    // closely as possible.  Mostly, that's border widths and shadows.  Size
    // is probably set in some other way, but this looked close enough.
    add_button(Gtk::Stock::CLOSE,Gtk::RESPONSE_CLOSE);
    set_default_size(525,320);
    set_border_width(5);

    get_vbox()->pack_start(make_scrolled_text(license));
}

CreditsBox::CreditsBox(Gtk::Window& parent,
                       std::vector<Glib::ustring>& authors,
                       std::vector<Glib::ustring>& translators
)
    : AboutBoxChild(parent, _("Credits"))
{
    // This attempts to emulate the AboutDialog Credits dialog settings as
    // closely as possible.  Mostly, that's border widths and shadows.  Size
    // is probably set in some other way, but this looked close enough.
    add_button(Gtk::Stock::CLOSE,Gtk::RESPONSE_CLOSE);
    set_default_size(360,260);
    set_border_width(5);

    _notebook.set_border_width(5);
    get_vbox()->pack_start(_notebook);

    Glib::ustring contents;

    flatten_vector(authors,contents);
    if (contents != "")
        _notebook.append_page(make_scrolled_text(contents),_("Authors"));

    flatten_vector(translators,contents);
    if (contents != "")
        _notebook.append_page(make_scrolled_text(contents),_("Translators"));
}

void
CreditsBox::flatten_vector(std::vector<Glib::ustring>& list,
                           Glib::ustring& string)
{
    std::vector<Glib::ustring>::iterator iter;

    string = "";
    for (iter = list.begin(); iter != list.end(); iter++) {
        string += *iter + "\n";
    }
}

Gtk::ScrolledWindow&
AboutBoxChild::make_scrolled_text(Glib::ustring& contents)
{
    // This attempts to emulate the AboutDialog child dialog settings as
    // closely as possible.  Mostly, that's margin widths and shadows.  Size
    // is probably set in some other way, but this looked close enough.
    Gtk::ScrolledWindow * scrolled = new Gtk::ScrolledWindow(); 
    scrolled->set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC);
    scrolled->set_shadow_type(Gtk::SHADOW_IN);

    Gtk::TextView *textview = new Gtk::TextView();
    textview->set_editable(FALSE);
    textview->set_left_margin(10);
    textview->set_right_margin(10);
    textview->get_buffer()->set_text(contents);

    scrolled->add(*textview);

    return *scrolled;
}


void
AboutBoxChild::on_response(int response_id)
{
    switch (response_id) {
        case Gtk::RESPONSE_CLOSE: {
            hide();
            break;
        }
        default:
            break;
    }
}

AboutBox::AboutBox(Gtk::Widget& about_svg_view, gint width, gint height)
    : Gtk::Dialog(_("About Inkscape"))
{
    _license = NULL;
    _credits = NULL;


    add_button(_("_Credits"),    INKSCAPE_ABOUT_CREDITS);
    add_button(_("_License"),    INKSCAPE_ABOUT_LICENSE);
    add_button(Gtk::Stock::CLOSE,Gtk::RESPONSE_CLOSE);

    get_vbox()->add(about_svg_view);
    // allow window to shrink, but restore window size
    set_size_request(0,0);
    // "height" seems very broken... I'm cheating for now
    set_default_size(width,width-60);
}

void
AboutBox::on_response(int response_id)
{
    switch (response_id) {
        case INKSCAPE_ABOUT_CREDITS: {
            show_credits();
            break;
        }
        case INKSCAPE_ABOUT_LICENSE: {
            show_license();
            break;
        }
        case Gtk::RESPONSE_CLOSE: {
            if (_license) delete _license;
            if (_credits) delete _credits;
            delete this;
            break;
        }
        default:
            break;
    }
}

void
AboutBox::show_credits(void)
{
    if (!_credits) {
        // Authors
        std::vector<Glib::ustring> authors;
        authors.push_back("Josh Andler");
        authors.push_back("John Bintz");
        authors.push_back("Arpad Biro");
        authors.push_back("Daniel Borgmann");
        authors.push_back("Hans Breuer");
        authors.push_back("Nicu Buculei");
        authors.push_back("Bulia Byak");
        authors.push_back("Chema Celorio");
        authors.push_back("Johan Ceuppens");
        authors.push_back("Zbigniew Chyla");
        authors.push_back("Alexander Clausen");
        authors.push_back("John Cliff");
        authors.push_back("Kees Cook");
        authors.push_back("Ben Cromwell");
        authors.push_back("Robert Crosbie");
        authors.push_back("Jon Cruz");
        authors.push_back("Daniel Díaz");
        authors.push_back("Larry Doolittle");
        authors.push_back("Maxim V. Dziumanenko");
        authors.push_back("Danilo Egan");
        authors.push_back("Frank Felfe");
        authors.push_back("Andrew Fitzsimon");
        authors.push_back("Edward Flick");
        authors.push_back("Fred");
        authors.push_back("Ben Fowler");
        authors.push_back("Ted Gould");
        authors.push_back("Bryce Harrington");
        authors.push_back("Carl Hetherington");
        authors.push_back("Karl Ove Hufthammer");
        authors.push_back("Richard Hughes");
        authors.push_back("Nathan Hurst");
        authors.push_back("Thomas Ingham");
        authors.push_back("Bob Jamison");
        authors.push_back("Lauris Kaplinski");
        authors.push_back("Lynn Kerby");
        authors.push_back("Petr Kovar");
        authors.push_back("Raph Levien");
        authors.push_back("Nicklas Lindgren");
        authors.push_back("Vitaly Lipatov");
        authors.push_back("Colin Marquardt");
        authors.push_back("Dmitry G. Mastrukov");
        authors.push_back("Matiphas");
        authors.push_back("Michael Meeks");
        authors.push_back("Federico Mena");
        authors.push_back("MenTaLguY");
        authors.push_back("Aubanel Monnier");
        authors.push_back("Derek P. Moore");
        authors.push_back("Peter Moulder");
        authors.push_back("Jörg Müller");
        authors.push_back("Yukihiro Nakai");
        authors.push_back("Christian Neumair");
        authors.push_back("Andreas Nilsson");
        authors.push_back("Mitsuru Oka");
        authors.push_back("Jon Phillips");
        authors.push_back("Zdenko Podobny");
        authors.push_back("Alexandre Prokoudine");
        authors.push_back("Alexey Remizov");
        authors.push_back("Frederic Rodrigo");
        authors.push_back("Juarez Rudsatz");
        authors.push_back("Xavier Conde Rueda");
        authors.push_back("Christian Schaller");
        authors.push_back("Tom von Schwerdtner");
        authors.push_back("Shivaken");
        authors.push_back("Boštjan Špetič");
        authors.push_back("Aaron Spike");
        authors.push_back("Kaushik Sridharan");
        authors.push_back("Ralf Stephan");
        authors.push_back("Dariusz Stojek");
        authors.push_back("Pat Suwalski");
        authors.push_back("Adib Taraben");
        authors.push_back("David Turner");
        authors.push_back("Aleksandar Urosevic");
        authors.push_back("Lucas Vieites");
        authors.push_back("Michael Wybrow");
        authors.push_back("Daniel Yacob");
        authors.push_back("David Yip");
        authors.push_back("Masatake Yamato");

        // Translators
        std::vector<Glib::ustring> translators;
        translators.push_back("Adib Taraben <theadib@yahoo.com>, 2004.");
        translators.push_back("Agradecimientos: zert, softcatala, 2002-2003");
        translators.push_back("Alastair McKinstry <mckinstry@computer.org>, 2000.");
        translators.push_back("Aleksandar Urošević <urke@users.sourceforge.net>");
        translators.push_back("Alessio Frusciante <algol@firenze.linux.it>, 2002, 2003.");
        translators.push_back("Alexandre Prokoudine <alexandre.prokoudine@gmail.com>, 2005.");
        translators.push_back("Alexey Remizov <alexey@remizov.pp.ru>, 2004.");
        translators.push_back("Álvaro Lopes <alvieboy@alvie.com>, 2001, 2002");
        translators.push_back("Andreas Hyden <a.hyden@cyberpoint.se>, 2000.");
        translators.push_back("Arman Aksoy <armish@linux-sevenler.de>, 2003.");
        translators.push_back("Arpad Biro <biro_arpad@yahoo.com>, 2004, 2005.");
        translators.push_back("Benedikt Roth <Benedikt.Roth@gmx.net>, 2000");
        translators.push_back("Boštjan Špetič <igzebedze@cyberpipe.org>, 2004, 2005.");
        translators.push_back("Brisa Francesco <fbrisa@yahoo.it>, 2000.");
        translators.push_back("bulia byak <buliabyak@users.sf.net>, 2004.");
        translators.push_back("Christian Meyer <chrisime@gnome.org>, 2000-2002.");
        translators.push_back("Christian Neumair <chris@gnome-de.org>, 2002, 2003.");
        translators.push_back("Christian Rose <menthos@menthos.com>, 2000, 2001, 2002, 2003.");
        translators.push_back("Christophe Merlet (RedFox) <redfox@redfoxcenter.org>, 2000-2002.");
        translators.push_back("Colin Marquardt <colin@marquardt-home.de>, 2004, 2005.");
        translators.push_back("Daniel Díaz <yosoy@danieldiaz.org>, 2004");
        translators.push_back("Александар Урошевић <urke@users.sourceforge.net>");
        translators.push_back("Didier Conchaudron <conchaudron@free.fr>, 2003.");
        translators.push_back("Duarte Loreto <happyguy_pt@hotmail.com> 2002,2003 (Maintainer)");
        translators.push_back("Fatih Demir <kabalak@gtranslator.org>, 2000.");
        translators.push_back("Francesc Dorca <f.dorca@filnet.es>, 2003. Traducció sodipodi.");
        translators.push_back("Francisco Javier F. Serrador <serrador@arrakis.es>, 2003");
        translators.push_back("Francisco Javier F. Serrador <serrador@arrakis.es>, 2003.");
        translators.push_back("Francisco Xosé Vázquez Grandal <fxvazquez@arrakis.es>, 2001.");
        translators.push_back("Frederic Rodrigo <f.rodrigo free.fr>, 2004-2005.");
        translators.push_back("Ge'ez Frontier Foundation <locales@geez.org>, 2002.");
        translators.push_back("Jörg Müller <jfm@ram-brand.de>, 2005.");
        translators.push_back("Jeroen van der Vegt <ajvdvegt (at) 123mail.org>, 2003, 2005.");
        translators.push_back("Jose Antonio Salgueiro Aquino <developer@telefonica.net>, 2003");
        translators.push_back("Jose Antonio Salgueiro Aquino <developer@telefonica.net>, 2003.");
        translators.push_back("Josef Vybiral <josef.vybiral@gmail.com>, 2005.");
        translators.push_back("Juarez Rudsatz <juarez@correio.com>, 2004");
        translators.push_back("Junichi Uekawa <dancer@debian.org>, 2002.");
        translators.push_back("Kai Lahmann <kailahmann@01019freenet.de>, 2000");
        translators.push_back("Karl Ove Hufthammer <karl@huftis.org>, 2004, 2005.");
        translators.push_back("Keld Simonsen <keld@dkuug.dk>, 2000-2001.");
        translators.push_back("Kjartan Maraas <kmaraas@gnome.org>, 2000-2002.");
        translators.push_back("Kjartan Maraas <kmaraas@online.no>, 2000.");
        translators.push_back("Lauris Kaplinski <lauris@ariman.ee>, 2000");
        translators.push_back("Luca Bruno <luca.br@uno.it>, 2005.");
        translators.push_back("Lucas Vieites Fariña<lucas@asixinformatica.com>, 2003-2005.");
        translators.push_back("Lucas Vieites <lucas@asixinformatica.com>, 2003");
        translators.push_back("Martin Srebotnjak, <miles@filmsi.net>, 2005.");
        translators.push_back("Masatake YAMATO <jet@gyve.org>, 2002.");
        translators.push_back("Matiphas <matiphas _a_ free _point_ fr>, 2004.");
        translators.push_back("Mattias Hultgren <mattias_hultgren@tele2.se>, 2005.");
        translators.push_back("Maxim Dziumanenko <mvd@mylinux.com.ua>, 2004");
        translators.push_back("Mitsuru Oka <oka326@parkcity.ne.jp>, 2002.");
        translators.push_back("Mitsuru Oka <oka@debian.or.jp>, 2001.");
        translators.push_back("Mufit Eribol <meribol@ere.com.tr>, 2000.");
        translators.push_back("Quico Llach <quico@softcatala.org>, 2000. Traducció sodipodi.");
        translators.push_back("Raymond Ostertag <raymond@linuxgraphic.org>, 2002-2003.");
        translators.push_back("shivaken <shivaken@owls-nest.net>, 2004.");
        translators.push_back("Simos Xenitellis <simos@hellug.gr>, 2001.");
        translators.push_back("Takeshi Aihana <aihana@muc.biglobe.ne.jp>, 2000-2001.");
        translators.push_back("Traducido por Jose Antonio Salgueiro <developer@telefonica.net>.");
        translators.push_back("Traducido por Jose Antonio Salgueiro <developer@telefonica.net>.");
        translators.push_back("translators@gnome.pl");
        translators.push_back("Valek Filippov <frob@df.ru>, 2000, 2003.");
        translators.push_back("Vincent van Adrighem <V.vanAdrighem@dirck.mine.nu>, 2003.");
        translators.push_back("Vital Khilko <dojlid@mova.org>, 2003");
        translators.push_back("Vitaly Lipatov <lav@altlinux.ru>, 2002, 2004.");
        translators.push_back("Wang Li <charlesw1234@163.com>, 2002");
        translators.push_back("Xavier Conde Rueda <xavi.conde@gmail.com>, 2004, 2005");
        translators.push_back("Yukihiro Nakai <nakai@gnome.gr.jp>, 2000, 2003.");
        translators.push_back("Yuri Syrota <rasta@renome.rovno.ua>, 2000.");
        translators.push_back("Zdenko Podobný <zdpo@mailbox.sk>, 2003, 2004.");

        _credits = new CreditsBox(*this, authors, translators);
    }
    _credits->show_all();
}

void
AboutBox::show_license(void)
{
    Glib::ustring gpl = 
"            GNU GENERAL PUBLIC LICENSE\n\
               Version 2, June 1991\n\
\n\
 Copyright (C) 1989, 1991 Free Software Foundation, Inc.\n\
     59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n\
 Everyone is permitted to copy and distribute verbatim copies\n\
 of this license document, but changing it is not allowed.\n\
\n\
                Preamble\n\
\n\
  The licenses for most software are designed to take away your\n\
freedom to share and change it.  By contrast, the GNU General Public\n\
License is intended to guarantee your freedom to share and change free\n\
software--to make sure the software is free for all its users.  This\n\
General Public License applies to most of the Free Software\n\
Foundation's software and to any other program whose authors commit to\n\
using it.  (Some other Free Software Foundation software is covered by\n\
the GNU Library General Public License instead.)  You can apply it to\n\
your programs, too.\n\
\n\
  When we speak of free software, we are referring to freedom, not\n\
price.  Our General Public Licenses are designed to make sure that you\n\
have the freedom to distribute copies of free software (and charge for\n\
this service if you wish), that you receive source code or can get it\n\
if you want it, that you can change the software or use pieces of it\n\
in new free programs; and that you know you can do these things.\n\
\n\
  To protect your rights, we need to make restrictions that forbid\n\
anyone to deny you these rights or to ask you to surrender the rights.\n\
These restrictions translate to certain responsibilities for you if you\n\
distribute copies of the software, or if you modify it.\n\
\n\
  For example, if you distribute copies of such a program, whether\n\
gratis or for a fee, you must give the recipients all the rights that\n\
you have.  You must make sure that they, too, receive or can get the\n\
source code.  And you must show them these terms so they know their\n\
rights.\n\
\n\
  We protect your rights with two steps: (1) copyright the software, and\n\
(2) offer you this license which gives you legal permission to copy,\n\
distribute and/or modify the software.\n\
\n\
  Also, for each author's protection and ours, we want to make certain\n\
that everyone understands that there is no warranty for this free\n\
software.  If the software is modified by someone else and passed on, we\n\
want its recipients to know that what they have is not the original, so\n\
that any problems introduced by others will not reflect on the original\n\
authors' reputations.\n\
\n\
  Finally, any free program is threatened constantly by software\n\
patents.  We wish to avoid the danger that redistributors of a free\n\
program will individually obtain patent licenses, in effect making the\n\
program proprietary.  To prevent this, we have made it clear that any\n\
patent must be licensed for everyone's free use or not licensed at all.\n\
\n\
  The precise terms and conditions for copying, distribution and\n\
modification follow.\n\
\n\
            GNU GENERAL PUBLIC LICENSE\n\
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION\n\
\n\
  0. This License applies to any program or other work which contains\n\
a notice placed by the copyright holder saying it may be distributed\n\
under the terms of this General Public License.  The \n\"Program\n\", below,\n\
refers to any such program or work, and a \n\"work based on the Program\n\"\n\
means either the Program or any derivative work under copyright law:\n\
that is to say, a work containing the Program or a portion of it,\n\
either verbatim or with modifications and/or translated into another\n\
language.  (Hereinafter, translation is included without limitation in\n\
the term \n\"modification\n\".)  Each licensee is addressed as \n\"you\n\".\n\
\n\
Activities other than copying, distribution and modification are not\n\
covered by this License; they are outside its scope.  The act of\n\
running the Program is not restricted, and the output from the Program\n\
is covered only if its contents constitute a work based on the\n\
Program (independent of having been made by running the Program).\n\
Whether that is true depends on what the Program does.\n\
\n\
  1. You may copy and distribute verbatim copies of the Program's\n\
source code as you receive it, in any medium, provided that you\n\
conspicuously and appropriately publish on each copy an appropriate\n\
copyright notice and disclaimer of warranty; keep intact all the\n\
notices that refer to this License and to the absence of any warranty;\n\
and give any other recipients of the Program a copy of this License\n\
along with the Program.\n\
\n\
You may charge a fee for the physical act of transferring a copy, and\n\
you may at your option offer warranty protection in exchange for a fee.\n\
\n\
  2. You may modify your copy or copies of the Program or any portion\n\
of it, thus forming a work based on the Program, and copy and\n\
distribute such modifications or work under the terms of Section 1\n\
above, provided that you also meet all of these conditions:\n\
\n\
    a) You must cause the modified files to carry prominent notices\n\
    stating that you changed the files and the date of any change.\n\
\n\
    b) You must cause any work that you distribute or publish, that in\n\
    whole or in part contains or is derived from the Program or any\n\
    part thereof, to be licensed as a whole at no charge to all third\n\
    parties under the terms of this License.\n\
\n\
    c) If the modified program normally reads commands interactively\n\
    when run, you must cause it, when started running for such\n\
    interactive use in the most ordinary way, to print or display an\n\
    announcement including an appropriate copyright notice and a\n\
    notice that there is no warranty (or else, saying that you provide\n\
    a warranty) and that users may redistribute the program under\n\
    these conditions, and telling the user how to view a copy of this\n\
    License.  (Exception: if the Program itself is interactive but\n\
    does not normally print such an announcement, your work based on\n\
    the Program is not required to print an announcement.)\n\
\n\
These requirements apply to the modified work as a whole.  If\n\
identifiable sections of that work are not derived from the Program,\n\
and can be reasonably considered independent and separate works in\n\
themselves, then this License, and its terms, do not apply to those\n\
sections when you distribute them as separate works.  But when you\n\
distribute the same sections as part of a whole which is a work based\n\
on the Program, the distribution of the whole must be on the terms of\n\
this License, whose permissions for other licensees extend to the\n\
entire whole, and thus to each and every part regardless of who wrote it.\n\
\n\
Thus, it is not the intent of this section to claim rights or contest\n\
your rights to work written entirely by you; rather, the intent is to\n\
exercise the right to control the distribution of derivative or\n\
collective works based on the Program.\n\
\n\
In addition, mere aggregation of another work not based on the Program\n\
with the Program (or with a work based on the Program) on a volume of\n\
a storage or distribution medium does not bring the other work under\n\
the scope of this License.\n\
\n\
  3. You may copy and distribute the Program (or a work based on it,\n\
under Section 2) in object code or executable form under the terms of\n\
Sections 1 and 2 above provided that you also do one of the following:\n\
\n\
    a) Accompany it with the complete corresponding machine-readable\n\
    source code, which must be distributed under the terms of Sections\n\
    1 and 2 above on a medium customarily used for software interchange; or,\n\
\n\
    b) Accompany it with a written offer, valid for at least three\n\
    years, to give any third party, for a charge no more than your\n\
    cost of physically performing source distribution, a complete\n\
    machine-readable copy of the corresponding source code, to be\n\
    distributed under the terms of Sections 1 and 2 above on a medium\n\
    customarily used for software interchange; or,\n\
\n\
    c) Accompany it with the information you received as to the offer\n\
    to distribute corresponding source code.  (This alternative is\n\
    allowed only for noncommercial distribution and only if you\n\
    received the program in object code or executable form with such\n\
    an offer, in accord with Subsection b above.)\n\
\n\
The source code for a work means the preferred form of the work for\n\
making modifications to it.  For an executable work, complete source\n\
code means all the source code for all modules it contains, plus any\n\
associated interface definition files, plus the scripts used to\n\
control compilation and installation of the executable.  However, as a\n\
special exception, the source code distributed need not include\n\
anything that is normally distributed (in either source or binary\n\
form) with the major components (compiler, kernel, and so on) of the\n\
operating system on which the executable runs, unless that component\n\
itself accompanies the executable.\n\
\n\
If distribution of executable or object code is made by offering\n\
access to copy from a designated place, then offering equivalent\n\
access to copy the source code from the same place counts as\n\
distribution of the source code, even though third parties are not\n\
compelled to copy the source along with the object code.\n\
\n\
  4. You may not copy, modify, sublicense, or distribute the Program\n\
except as expressly provided under this License.  Any attempt\n\
otherwise to copy, modify, sublicense or distribute the Program is\n\
void, and will automatically terminate your rights under this License.\n\
However, parties who have received copies, or rights, from you under\n\
this License will not have their licenses terminated so long as such\n\
parties remain in full compliance.\n\
\n\
  5. You are not required to accept this License, since you have not\n\
signed it.  However, nothing else grants you permission to modify or\n\
distribute the Program or its derivative works.  These actions are\n\
prohibited by law if you do not accept this License.  Therefore, by\n\
modifying or distributing the Program (or any work based on the\n\
Program), you indicate your acceptance of this License to do so, and\n\
all its terms and conditions for copying, distributing or modifying\n\
the Program or works based on it.\n\
\n\
  6. Each time you redistribute the Program (or any work based on the\n\
Program), the recipient automatically receives a license from the\n\
original licensor to copy, distribute or modify the Program subject to\n\
these terms and conditions.  You may not impose any further\n\
restrictions on the recipients' exercise of the rights granted herein.\n\
You are not responsible for enforcing compliance by third parties to\n\
this License.\n\
\n\
  7. If, as a consequence of a court judgment or allegation of patent\n\
infringement or for any other reason (not limited to patent issues),\n\
conditions are imposed on you (whether by court order, agreement or\n\
otherwise) that contradict the conditions of this License, they do not\n\
excuse you from the conditions of this License.  If you cannot\n\
distribute so as to satisfy simultaneously your obligations under this\n\
License and any other pertinent obligations, then as a consequence you\n\
may not distribute the Program at all.  For example, if a patent\n\
license would not permit royalty-free redistribution of the Program by\n\
all those who receive copies directly or indirectly through you, then\n\
the only way you could satisfy both it and this License would be to\n\
refrain entirely from distribution of the Program.\n\
\n\
If any portion of this section is held invalid or unenforceable under\n\
any particular circumstance, the balance of the section is intended to\n\
apply and the section as a whole is intended to apply in other\n\
circumstances.\n\
\n\
It is not the purpose of this section to induce you to infringe any\n\
patents or other property right claims or to contest validity of any\n\
such claims; this section has the sole purpose of protecting the\n\
integrity of the free software distribution system, which is\n\
implemented by public license practices.  Many people have made\n\
generous contributions to the wide range of software distributed\n\
through that system in reliance on consistent application of that\n\
system; it is up to the author/donor to decide if he or she is willing\n\
to distribute software through any other system and a licensee cannot\n\
impose that choice.\n\
\n\
This section is intended to make thoroughly clear what is believed to\n\
be a consequence of the rest of this License.\n\
\n\
  8. If the distribution and/or use of the Program is restricted in\n\
certain countries either by patents or by copyrighted interfaces, the\n\
original copyright holder who places the Program under this License\n\
may add an explicit geographical distribution limitation excluding\n\
those countries, so that distribution is permitted only in or among\n\
countries not thus excluded.  In such case, this License incorporates\n\
the limitation as if written in the body of this License.\n\
\n\
  9. The Free Software Foundation may publish revised and/or new versions\n\
of the General Public License from time to time.  Such new versions will\n\
be similar in spirit to the present version, but may differ in detail to\n\
address new problems or concerns.\n\
\n\
Each version is given a distinguishing version number.  If the Program\n\
specifies a version number of this License which applies to it and \n\"any\n\
later version\n\", you have the option of following the terms and conditions\n\
either of that version or of any later version published by the Free\n\
Software Foundation.  If the Program does not specify a version number of\n\
this License, you may choose any version ever published by the Free Software\n\
Foundation.\n\
\n\
  10. If you wish to incorporate parts of the Program into other free\n\
programs whose distribution conditions are different, write to the author\n\
to ask for permission.  For software which is copyrighted by the Free\n\
Software Foundation, write to the Free Software Foundation; we sometimes\n\
make exceptions for this.  Our decision will be guided by the two goals\n\
of preserving the free status of all derivatives of our free software and\n\
of promoting the sharing and reuse of software generally.\n\
\n\
                NO WARRANTY\n\
\n\
  11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY\n\
FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN\n\
OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES\n\
PROVIDE THE PROGRAM \n\"AS IS\n\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED\n\
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF\n\
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS\n\
TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE\n\
PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,\n\
REPAIR OR CORRECTION.\n\
\n\
  12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING\n\
WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR\n\
REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,\n\
INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING\n\
OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED\n\
TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY\n\
YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER\n\
PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE\n\
POSSIBILITY OF SUCH DAMAGES.\n\
\n\
             END OF TERMS AND CONDITIONS\n\
\n\
        How to Apply These Terms to Your New Programs\n\
\n\
  If you develop a new program, and you want it to be of the greatest\n\
possible use to the public, the best way to achieve this is to make it\n\
free software which everyone can redistribute and change under these terms.\n\
\n\
  To do so, attach the following notices to the program.  It is safest\n\
to attach them to the start of each source file to most effectively\n\
convey the exclusion of warranty; and each file should have at least\n\
the \n\"copyright\n\" line and a pointer to where the full notice is found.\n\
\n\
    <one line to give the program's name and a brief idea of what it does.>\n\
    Copyright (C) <year>  <name of author>\n\
\n\
    This program is free software; you can redistribute it and/or modify\n\
    it under the terms of the GNU General Public License as published by\n\
    the Free Software Foundation; either version 2 of the License, or\n\
    (at your option) any later version.\n\
\n\
    This program is distributed in the hope that it will be useful,\n\
    but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
    GNU General Public License for more details.\n\
\n\
    You should have received a copy of the GNU General Public License\n\
    along with this program; if not, write to the Free Software\n\
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n\
\n\
\n\
Also add information on how to contact you by electronic and paper mail.\n\
\n\
If the program is interactive, make it output a short notice like this\n\
when it starts in an interactive mode:\n\
\n\
    Gnomovision version 69, Copyright (C) year  name of author\n\
    Gnomovision comes with ABSOLUTELY NO WARRANTY; for details type `show w'.\n\
    This is free software, and you are welcome to redistribute it\n\
    under certain conditions; type `show c' for details.\n\
\n\
The hypothetical commands `show w' and `show c' should show the appropriate\n\
parts of the General Public License.  Of course, the commands you use may\n\
be called something other than `show w' and `show c'; they could even be\n\
mouse-clicks or menu items--whatever suits your program.\n\
\n\
You should also get your employer (if you work as a programmer) or your\n\
school, if any, to sign a \n\"copyright disclaimer\n\" for the program, if\n\
necessary.  Here is a sample; alter the names:\n\
\n\
  Yoyodyne, Inc., hereby disclaims all copyright interest in the program\n\
  `Gnomovision' (which makes passes at compilers) written by James Hacker.\n\
\n\
  <signature of Ty Coon>, 1 April 1989\n\
  Ty Coon, President of Vice\n\
\n\
This General Public License does not permit incorporating your program into\n\
proprietary programs.  If your program is a subroutine library, you may\n\
consider it more useful to permit linking proprietary applications with the\n\
library.  If this is what you want to do, use the GNU Library General\n\
Public License instead of this License.\n\
";

    if (!_license) {
        _license = new LicenseBox(*this,gpl);
    }
    _license->show_all();
}


} // namespace Dialog
} // namespace UI
} // namespace Inkscape

/* 
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
