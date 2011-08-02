<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet
      version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform">


<xsl:output method="xml" indent="yes" />
  
    <xsl:template match="coords">

    <svg xmlns="http://www.w3.org/2000/svg"
        width="2"
        height="2"
    >

        <path xsl:use-attribute-sets="pathCoords">
        </path>

        </svg>
    </xsl:template>


    <xsl:attribute-set name="pathCoords">
        <xsl:attribute name="style">fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:0.5;stroke-opacity:1.0</xsl:attribute>
        <xsl:attribute name="d">

            <xsl:text>M </xsl:text>
            <xsl:apply-templates select="point"/>

        </xsl:attribute>
    </xsl:attribute-set>


    <xsl:template match="point">
<!--    <xsl:text>L </xsl:text>-->
        <xsl:apply-templates select="x"/>
        <xsl:text> </xsl:text>

        <xsl:apply-templates select="y"/>
        <xsl:text> L </xsl:text>

    </xsl:template>


    <xsl:template match="x">
        <xsl:value-of select="." />
    </xsl:template>
    
    <xsl:template match="y">
        <xsl:value-of select="." />
    </xsl:template>

</xsl:stylesheet>
