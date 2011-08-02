<?xml version="1.0"?>
<xsl:stylesheet version="1.0" 
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="xml" indent="yes" encoding="ISO-8859-1"/>
  <xsl:template match="/">
    <svg xmlns="http://www.w3.org/2000/svg">
        <xsl:apply-templates />
    </svg>
  </xsl:template>
  <xsl:template match="Document">
      <xsl:apply-templates />
  </xsl:template>
  <xsl:template match="Folder">
      <xsl:apply-templates />
  </xsl:template>
  <xsl:template match="Placemark">
      <xsl:apply-templates />
  </xsl:template>
  <xsl:template match="LineString">
      <xsl:apply-templates />
  </xsl:template>
    <xsl:template match="coordinates">
      <path xsl:use-attribute-sets="pathInfo">
        <xsl:value-of select="."/>
    </xsl:template>
    <xsl:attribute-set name="pathInfo">
      <xsl:attribute name="style">fill:none;fill-rule:evenodd;stroke:#000000;stroke-width:0.5;stroke-opacity:1.0</xsl:attribute>
      <xsl:attribute name="d">M 0.10176603810611,18.011610313383 L 600,370.03314165421 L 0,0</xsl:attribute>
      <xsl:attribute name="id">Sans titre - Trajet_2</xsl:attribute>
    </xsl:attribute-set>
</xsl:stylesheet>
