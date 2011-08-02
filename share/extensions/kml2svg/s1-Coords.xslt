<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet
    version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="xml" indent="yes" />
 
  <xsl:template match="/">
<!--      <svg xmlns="http://www.w3.org/2000/svg">-->

      <coords>
        <xsl:apply-templates select="kml/Document/Folder/Placemark/LineString"/>
      </coords>
        
<!--      </svg>-->
  </xsl:template>
 
  <xsl:template match="LineString">
      <xsl:apply-templates select="coordinates"/>
  </xsl:template>
 
    <xsl:template match="coordinates">
        <xsl:call-template name="SplitLinks">
            <xsl:with-param name="pLinks" select="text()"/>
        </xsl:call-template>
    </xsl:template>

    <!-- recursive template for splitting links (text|url) -->
    <xsl:template name="SplitLinks">
        <xsl:param name="pLinks"/>
        <!-- how many seperators are there in the string... -->
        <xsl:variable name="vCountSeperators" select="string-length($pLinks) - string-length(translate($pLinks,' ',''))"/>
        <xsl:choose>
            <xsl:when test="$vCountSeperators &gt;= 2">
                <point>
                    <xsl:value-of select="substring-before($pLinks,' ')"/>
                </point>
<!--                <point>
                    <xsl:value-of select="substring-before(substring-after($pLinks,' '),' ')"/>
                </point>
                <xsl:call-template name="SplitLinks">
                    <xsl:with-param name="pLinks" select="substring-after(substring-after($pLinks,' '),' ')"/>
                </xsl:call-template>-->

                <xsl:call-template name="SplitLinks">
                    <xsl:with-param name="pLinks" select="substring-after($pLinks,' ')"/>
                </xsl:call-template>

            </xsl:when>
        </xsl:choose>
    
  </xsl:template>
</xsl:stylesheet>

