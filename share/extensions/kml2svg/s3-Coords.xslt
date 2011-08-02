<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet
      version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform">


<!--<xsl:import href="math.xsl" />-->

<!--   <import href="functions/log/math.log.xsl"/>
   <import href="functions/random/math.random.xsl"/>
   <import href="functions/sin/math.sin.xsl"/>
   <import href="functions/cos/math.cos.xsl"/>
   <import href="functions/tan/math.tan.xsl"/>-->


<xsl:output method="xml" indent="yes" />
  
    <xsl:template match="coords">

        <coords>
<!--            <xsl:for-each select="point">-->

                <xsl:apply-templates select="point" />
<!--            </xsl:for-each>-->
        </coords>
    </xsl:template>


  <xsl:template match="point">
<!--                      <xsl:apply-templates select="point" />-->

                <point>
    <xsl:apply-templates select="longitude"/>

                  <xsl:apply-templates select="latitude"/>

                </point>
  </xsl:template>


  <xsl:template match="longitude">
    <x>
        <xsl:value-of select=".  * 3.1415926 div  180.0" />
    </x>
  </xsl:template>
      	y = -log ( tan (pi() / 4 + φ / 2) , 2.718282 );
  <xsl:template match="latitude">
    <y>
        <xsl:value-of select=" .  * 3.1415926 div  180.0" />
<!--        <xsl:value-of select=" math:log ( (math:tan (3.1415926 div 4 + (.  * 3.1415926 div  180.0) div 2)) , 2.718282 )" />-->
    </y>
  </xsl:template>

</xsl:stylesheet>
