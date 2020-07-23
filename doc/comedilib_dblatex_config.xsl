<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:fo="http://www.w3.org/1999/XSL/Format"
	version="1.0">
	<xsl:param name="funcsynopsis.decoration" select="1"/>
	<xsl:param name="funcsynopsis.style">ansi</xsl:param>
	<xsl:param name="function.parens" select="1"/>

	<!-- Put parameter names in italics -->
	<xsl:template match="paramdef/parameter">
		<xsl:choose>
			<xsl:when test="$funcsynopsis.decoration != 0">
				<xsl:text>\textit{</xsl:text>
				<xsl:apply-templates/>
				<xsl:text>}</xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:apply-templates/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
</xsl:stylesheet>
