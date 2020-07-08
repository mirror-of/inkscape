#/usr/bin/python3
import sys, os, glob
iconsize = 20
resolution = 900
columns = 5
PATH = os.path.join(os.path.dirname(os.path.realpath(__file__)))
for filename in os.listdir(PATH):
    fullfile = os.path.join(PATH, filename)
    if os.path.isdir(fullfile) and filename != "application":
        for filename2 in os.listdir(fullfile):
            fullfile2 = os.path.join(fullfile, filename2)
            if os.path.isdir(fullfile2):
                f = open(os.path.join(PATH ,'theme-variant.svg'), 'r')
                themesvg = f.read()
                f.close()
                img = ""
                cols = 3
                x = -1
                y = 0
                counter = -1
                for filename3 in os.listdir(fullfile2):
                    fullfile3 = os.path.join(fullfile2, filename3)
                    if filename3 == "actions":
                        for filename4 in os.listdir(fullfile3):
                            counter += 1
                            if counter%columns == 0:
                                y += iconsize + 5
                            x = (counter % columns) * (iconsize + 5)
                            image = filename + "/" + filename2 + "/" + filename3 + "/" + filename4
                            img += '<image xlink:href="' + image + '" y="' + str(y) + '" x="'+ str(x) +'" preserveAspectRatio="none" inkscape:svg-dpi="'+str(resolution)+'" width="'+str(iconsize)+'" height="'+str(iconsize)+'" style="image-rendering:optimizeQuality" id="image '+ str(x) +'_'+ str(y)+'" />'
                themefile = os.path.join(PATH ,filename + "-" + filename2 + ".svg")
                if os.path.isfile(themefile):
                    os.remove(themefile)
                f = open(themefile, 'w')
                f.write(themesvg.replace("</g>",img + "</g>"))
                f.close()
