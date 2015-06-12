#include "file_resources.hpp"

Html_file htmlfile__index_html {
    "index.html",
    "/",
    "3107\n",
    "Tue, 26 May 2015 15:09:45 GMT",
    "<!DOCTYPE html>\n"
    "\n"
    "<html>\n"
    "<head>\n"
    "	<meta charset=\"utf-8\">\n"
    "	<title>Page layout 2</title>\n"
    "	<style>\n"
    "		body {\n"
    "			font: 80% arial, helvetica, sans-serif;\n"
    "			margin: 0;\n"
    "		}\n"
    "		\n"
    "		#navigation {\n"
    "			position: absolute;\n"
    "			left: 0;\n"
    "			width: 15em;\n"
    "		}\n"
    "\n"
    "		#content {\n"
    "			margin-left: 15em;\n"
    "		}\n"
    "		\n"
    "		\n"
    "	</style>\n"
    "</head>\n"
    "\n"
    "<body>\n"
    "\n"
    "	<div id=\"navigation\">\n"
    "\n"
    "		<ul>\n"
    "			<li><a href=\"/test_file_root.html\">Test file</a></li>\n"
    "            <li><a href=\"/test/directory/updates/file.html\">Updates</a></li>\n"
    "			<li><a href=\"\">Natural Selection</a></li>\n"
    "			<li><a href=\"\">Genetics</a></li>\n"
    "		</ul>\n"
    "\n"
    "		<ul>\n"
    "			<li><a href=\"\">Erasmus Darwin</a></li>\n"
    "			<li><a href=\"\">Lamarck</a></li>\n"
    "			<li><a href=\"\">Charles Darwin</a></li>\n"
    "			<li><a href=\"\">Wallace</a></li>\n"
    "			<li><a href=\"\">Dawkins</a></li>\n"
    "		</ul>\n"
    "\n"
    "	</div>\n"
    "\n"
    "	<div id=\"content\">\n"
    "\n"
    "		<h2>On the Origin of The Origin</h2>\n"
    "		<p>Darwin's father was dead set on his son becoming a cleric but even though the young rapscallion began to study theology he found worms much more interesting.</p>\n"
    "		<p>When a lonely man with a moustache asked Darwin to ride with him on his boat named after a dog, Darwin agreed and set off around the world fiddling with wildlife.</p>\n"
    "		<p>Some say it was a load of birds from a bunch of islands hundreds of miles off the Ecuadorean coast that inspired his now widely accepted explanation of the mechanism of evolution. \"Why does that bird on that island have a beak like that while that finch on that island has a beak like that? AHA! I've got it! Natural Selection!\" he thought. It wasn't actually quite like that, but, y'know, it's a fine, popular romantic myth.</p>\n"
    "\n"
    "		<h2>The Origin</h2>\n"
    "		<p>Upon return from his jaunt, Charles chronicled his escapades (as <em>The Voyage of the Beagle</em>) and got a bit carried away with barnacles, although his theory of evolution was always ticking away in the back of his mind.</p>\n"
    "		<p>Some 20 or so year after he returned to England, a Welsh naturalist by the name of Wallace popped up with a similar idea to Darwin's grand theory. Darwin got a move on.</p>\n"
    "		<p>In 1858 a paper jointly attributed to Darwin and Wallace was presented to the Linnean Society of London that sent rumbles across the establishment and really ticked off a fair few people. The next year saw the publication of Darwin's 500-page \"abstract\" - <em>On The Origin of Species by Means of Natural Selection or the Preservation of Favoured Races In The Struggle For Life</em> (or <abbr>OTOOSBMONSOTPOFRITSFL</abbr> for short).</p>\n"
    "		<p>Darwin, already a prominent arc in scientific circles, was propelled into megastardom.</p>\n"
    "\n"
    "		<h2>After The Origin</h2>\n"
    "		<p>Chuck D revised The Origin five times, toning down each one a bit more than the one before it, partly to appease his religious wife. Who also happened to be his cousin. But some years later he'd had enough of trying to disguise the logical conclusion that humans are descended from the same common ancestor as all other animals and his third classic, <em>The Descent of Man</em>, was published and <em>really</em> pissed off the religious establishment.</p>\n"
    "\n"
    "	</div>\n"
    "\n"
    "</body>\n"
    "\n"
    "</html>\n"
};

Html_file htmlfile__test_file_root_html {
    "test_file_root.html",
    "/",
    "363\n",
    "Tue, 26 May 2015 15:08:17 GMT",
    "<html>\n"
    "    <body>\n"
    "        <p>\n"
    "    Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod\n"
    "tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At\n"
    "vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren,\n"
    "no sea takimata sanctus est Lorem ipsum dolor sit amet.\n"
    "        </p>\n"
    "    </body>\n"
    "</html>\n"
};

Html_file htmlfile__test_test_file_html {
    "test_file.html",
    "/test",
    "0\n",
    "Thu, 21 May 2015 16:50:08 GMT",
};

Html_file htmlfile__test_directory_updates_file_html {
    "file.html",
    "/test/directory/updates",
    "127\n",
    "Tue, 26 May 2015 15:09:10 GMT",
    "<html>\n"
    "    <body>\n"
    "        <p>\n"
    "        <h1>For example, here should be data from sensors.</h1>\n"
    "        </p>\n"
    "    </body>\n"
    "</html>\n"
};

Html_file htmlfile__404 {
    "404.html",
    "/",
    "94\n",
    "Tue, 26 May 2015 15:09:10 GMT",
    "<html>\n"
    "    <body>\n"
    "        <p>\n"
    "        <h1>404 Not Found</h1>\n"
    "        </p>\n"
    "    </body>\n"
    "</html>\n"
};
std::unordered_map<std::string, Html_file*> populate_file_umap()
{
    std::unordered_map<std::string, Html_file*> file_hash_table;
    file_hash_table["/"]                                 = &htmlfile__index_html;
    file_hash_table["/index.html"]                       = &htmlfile__index_html;
    file_hash_table["/test_file_root.html"]              = &htmlfile__test_file_root_html;
    file_hash_table["/test/directory/updates/file.html"] = &htmlfile__test_directory_updates_file_html;
    file_hash_table["/404.html"] = &htmlfile__404;
    return file_hash_table;
}
