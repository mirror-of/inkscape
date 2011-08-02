<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet
      version="1.0"
      xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="xml" indent="yes" />

    <xsl:template match="coords">
        <coords>

            <xsl:for-each select="point">
                <point>
                    <longitude>
                      <xsl:value-of select="substring-before(.,',')"/>
                    </longitude>
                    <latitude>
                      <xsl:value-of select="substring-before(substring-after(.,','),',')"/>
                    </latitude>

                </point>
            </xsl:for-each>

        </coords>
    </xsl:template>
</xsl:stylesheet>
